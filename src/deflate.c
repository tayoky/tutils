#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <tutils.h>

// little lib to deflate/inflate

struct bits_stream {
	FILE *file;
	uint32_t buffer;
	size_t bits_left;
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
	uint32_t bits;
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

int handle_fixed(struct bits_stream *in, FILE *out) {
	for (;;) {
		uint16_t symbol = get_fixed_symbol(in);
		if (symbol <= 255) {
			fputc(symbol, out);
			continue;
		}
		error("TODO");
	}
	return 0;
}

int deflate(FILE *in_file, FILE *out) {

	struct bits_stream in = {
		.file = in_file,
		.bits_left = 0,
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
				fputc(byte, out);
			}
			break;
		case 1:
			// fixed huffman
			handle_fixed(&in, out);
			break;
		case 2:
			error("TODO : dynamic huffman");
			break;
		}
	}
	return 0;
}
