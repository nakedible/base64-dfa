/* decoder */
#define BASE64_INIT 7
#define BASE64_EOF 13
#define BASE64_INVALID 15
static const unsigned char base64d[] = {
  /* character value & type mapping; type is highest 3 bits */
  0x9f,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x5f,0x5f,0xff,0xff,0x5f,0xff,0xff, // 00..0f
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, // 10..1f
  0x5f,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x3e,0xff,0xff,0xff,0x3f, // 20..2f
  0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0xff,0xff,0xff,0x7f,0xff,0xff, // 30..3f
  0xff,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e, // 40..4f
  0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0xff,0xff,0xff,0xff,0xff, // 50..5f
  0xff,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28, // 60..6f
  0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0xff,0xff,0xff,0xff,0xff, // 70..7f
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, // 80..8f
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, // 90..9f
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, // a0..af
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, // b0..bf
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, // c0..cf
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, // d0..df
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, // e0..ef
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, // f0..ff
  
  /* transitions: valid char * 2, whitespace, padding, eof, unused, unused, invalid */
  0x2,0x2,0x1,0xf,0xf,0xf,0xf,0xf, // 1 char input
  0x4,0x4,0x3,0x9,0xd,0xf,0xf,0xf, // 2 char input
  0x6,0x6,0x5,0xb,0xd,0xf,0xf,0xf, // 3 char input
  0x0,0x0,0x7,0xf,0xd,0xf,0xf,0xf, // 4 char input or null
  0xf,0xf,0x9,0xb,0xf,0xf,0xf,0xf, // 3 char padding
  0xf,0xf,0xb,0xd,0xd,0xf,0xf,0xf, // 4 char padding
  0xf,0xf,0xf,0xf,0xd,0xf,0xf,0xf, // eof
  0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf, // invalid
};
int base64_decode_char(int *state, int *val, char byte) {
  unsigned char type = base64d[((unsigned char)byte)];
  *state = base64d[256 + (*state>>1)*8 + (type >> 5)];
  *val = *state & 1 ? *val : *val << 8 | (type << (18 + (*state & 6)));
  return *state && !(*state & 1);
}
void base64_decode(const char *inbuf, char *outbuf, size_t outlen, size_t *resultlen) {
  size_t inpos = 0, outpos = 0;
  int state = BASE64_INIT, val = 0;
  while(outpos < outlen) {
    if (base64_decode_char(&state, &val, inbuf[inpos++])) {
      outbuf[outpos++] = (char)(val >> 24);
    } else if (state == BASE64_EOF) {
      *resultlen = outpos;
      return;
    } else if (state == BASE64_INVALID) {
      abort();
    }
  }
}

/* encoder */
static const char base64e[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
void base64_encode_chunk3(const char *in, char *out) {
  out[0] = base64e[(in[0] >> 2) & 63];
  out[1] = base64e[(in[0] << 4 | in[1] >> 4) & 63];
  out[2] = base64e[(in[1] << 2 | in[2] >> 6) & 63];
  out[3] = base64e[(in[2]) & 63];
}
void base64_encode_chunk2(const char *in, char *out) {
  out[0] = base64e[(in[0] >> 2) & 63];
  out[1] = base64e[(in[0] << 4 | in[1] >> 4) & 63];
  out[2] = base64e[(in[1] << 2) & 63];
  out[3] = '=';
}
void base64_encode_chunk1(const char *in, char *out) {
  out[0] = base64e[(in[0] >> 2) & 63];
  out[1] = base64e[(in[0] << 4) & 63];
  out[2] = '=';
  out[3] = '=';
}
void base64_encode(const char *inbuf, char *outbuf, size_t inlen) {
  size_t inpos = 0, outpos = 0;
  for (; (inpos + 2) < inlen; inpos += 3, outpos += 4)
    base64_encode_chunk3(inbuf+inpos, outbuf+outpos);
  if (inlen - inpos == 2) {
    base64_encode_chunk2(inbuf+inpos, outbuf+outpos);
    outpos += 4;
  } else if (inlen - inpos == 1) {
    base64_encode_chunk1(inbuf+inpos, outbuf+outpos);
    outpos += 4;
  }
  outbuf[outpos] = '\0';
}
