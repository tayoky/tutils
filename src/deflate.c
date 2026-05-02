#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <tutils.h>
#include <deflate.h>

// little lib to deflate/inflate

#define WINDOW_SIZE 32768

struct bits_stream {
	FILE *file;
	uint32_t buffer;
	size_t bits_left;
};

struct output {
	FILE *file;
	uint8_t window[WINDOW_SIZE];
	size_t window_ptr;
};

struct table_entry {
    uint16_t base;
    uint8_t extra_bits;
};

static const struct table_entry lenght_table[] = {
    {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0}, {10, 0}, // 257-264
    {11, 1}, {13, 1}, {15, 1}, {17, 1},                              // 265-268
    {19, 2}, {23, 2}, {27, 2}, {31, 2},                              // 269-272
    {35, 3}, {43, 3}, {51, 3}, {59, 3},                              // 273-276
    {67, 4}, {83, 4}, {99, 4}, {115, 4},                             // 277-280
    {131, 5}, {163, 5}, {195, 5}, {227, 5},                          // 281-284
    {258, 0}                                                         // 285
};

static const struct table_entry dist_table[] = {
    {1, 0}, {2, 0}, {3, 0}, {4, 0},                           // 0-3
    {5, 1}, {7, 1}, {9, 2}, {13, 2},                          // 4-7
    {17, 3}, {25, 3}, {33, 4}, {49, 4},                       // 8-11
    {65, 5}, {97, 5}, {129, 6}, {193, 6},                     // 12-15
    {257, 7}, {385, 7}, {513, 8}, {769, 8},                   // 16-19
    {1025, 9}, {1537, 9}, {2049, 10}, {3073, 10},             // 20-23
    {4097, 11}, {6145, 11}, {8193, 12}, {12289, 12},          // 24-27
    {16385, 13}, {24577, 13}                                  // 28-29
};

static uint8_t mask(size_t size) {
	return (1ULL << size) - 1;
}

static int read_buffer(struct bits_stream *stream) {
	if (fread(&stream->buffer, sizeof(stream->buffer), 1, stream->file) == 0) return 0;
	stream->bits_left = sizeof(stream->buffer) * CHAR_BIT;
	return 1;
}

static uint32_t read_bits(struct bits_stream *stream, size_t count) {
	uint32_t bits = 0;
	size_t offset = 0;
	while (count > 0) {
		if (stream->bits_left == 0) {
			read_buffer(stream);
		}
		if (count >= stream->bits_left) {
			bits |= stream->buffer << offset;
			count -= stream->bits_left;
			offset += stream->bits_left;
			stream->bits_left = 0;
		} else {
			bits |= (stream->buffer & mask(count)) << offset;
			stream->bits_left -= count;
			stream->buffer >>= count; 
			count = 0;
		}
	}
	return bits;
}

static int write_byte(struct output *out, uint8_t byte) {
	fputc(byte, out->file);
	out->window[out->window_ptr++] = byte;
	out->window_ptr %= WINDOW_SIZE;
	return 0;
}

static uint32_t reverse(uint32_t bits, size_t size) {
	uint32_t ret = 0;
	for (size_t i=0; i<size; i++) {
		ret = (ret << 1) | (bits & 1);
		bits >>= 1;
	}
	return ret;
}

static uint16_t get_fixed_symbol(struct bits_stream *in) {

	// handle 7 bits codes
	uint16_t data = reverse(read_bits(in, 7), 7);
	if (data <= 0b0010111) {
		return 256 + data;
	}

	// handle 8 bits codes
	data = (data << 1) | read_bits(in, 1);
	if (data <= 0b10111111) {
		return data - 0b00110000;
	}
	if (data <= 0b11000111) {
		return data - 0b11000000 + 280;
	}

	// handle 9 bits codes
	data = (data << 1) | read_bits(in, 1);
	return data - 0b110010000 + 144;
}

static uint16_t read_extra(struct bits_stream *in, const struct table_entry *entry) {
	uint16_t extra_bits = entry->extra_bits;
	return entry->base + read_bits(in, extra_bits);
}

static int handle_backward_ref(struct output *out, uint16_t len, uint16_t dist) {
	size_t ptr = out->window_ptr - dist + WINDOW_SIZE;
	for (uint16_t i=0; i<len; i++) {
		ptr %= WINDOW_SIZE;
		write_byte(out, out->window[ptr]);
		ptr++;
	}
	return 0;
}

int handle_fixed(struct bits_stream *in, struct output *out) {
	for (;;) {
		uint16_t symbol = get_fixed_symbol(in);
		if (symbol <= 255) {
			write_byte(out, symbol);
			continue;
		}
		if (symbol == 256) {
			// end of block symbol
			break;
		}
		uint16_t len_index = symbol - 257;
		uint16_t len = read_extra(in, &lenght_table[len_index]);
		uint8_t dist_index = reverse(read_bits(in, 5), 5);
		uint16_t dist = read_extra(in, &dist_table[dist_index]);

		handle_backward_ref(out, len, dist);
	}
	return 0;
}

int inflate(FILE *in_file, FILE *out_file) {
	// read header
	uint8_t header[10];
	if (!fread(&header, sizeof(header), 1, in_file)) {
not_gzip:
		error("not a gzip file");
		return -1;
	}
	if (header[0] != 0x1f || header[1] != 0x8b) {
		goto not_gzip;
	}
	// TODO : handle optionals fields

	struct bits_stream in = {
		.file = in_file,
	};

	struct output out = {
		.file = out_file,
	};

	// loop trough each block
	int final = 0;
	while (final == 0) {
		final = read_bits(&in, 1);
		int type = read_bits(&in, 2);

		if (type == 3) {
			error("invalid deflate block type : '3'");
			return -1;
		}
		switch (type) {
		case 0:;
			// no compression
			// empty the current byte
			if (in.bits_left % 8) {
				read_bits(&in, in.bits_left % 8);
			}
			uint16_t len = read_bits(&in, 16);
			uint16_t nlen = read_bits(&in, 16);
			if (len + nlen != 0xffff) {
				error("invalid deflate uncompress block");
			}
			for (size_t i=0; i<len; i++) {
				uint8_t byte = read_bits(&in, 8);
				write_byte(&out, byte);
			}
			break;
		case 1:
			// fixed huffman
			handle_fixed(&in, &out);
			break;
		case 2:
			error("TODO : dynamic huffman");
			break;
		}
	}
	return 0;
}
