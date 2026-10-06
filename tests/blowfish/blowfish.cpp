// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2026 OpenTS contributors
// See LICENSE.md for applicable additional terms and warranty disclaimers.

#include "blowfish.h"

#include <array>
#include <bit>
#include <cstdint>
#include <cstdio>

static_assert(sizeof(unsigned int) == 4);
static_assert(std::endian::native == std::endian::little);

namespace {

struct Vector {
	std::array<std::uint8_t, 8> Key;
	std::array<std::uint8_t, 8> Plain;
	std::array<std::uint8_t, 8> Cipher;
};

Vector const Vectors[] = {
	{{0, 0, 0, 0, 0, 0, 0, 0},
	 {0, 0, 0, 0, 0, 0, 0, 0},
	 {0x4E, 0xF9, 0x97, 0x45, 0x61, 0x98, 0xDD, 0x78}},
	{{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	 {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	 {0x51, 0x86, 0x6F, 0xD5, 0xB8, 0x5E, 0xCB, 0x8A}},
};

}


int main(void)
{
	int failures = 0;
	for (Vector const & vector : Vectors) {
		BlowfishEngine engine;
		engine.Submit_Key(vector.Key.data(), (int)vector.Key.size());
		std::array<std::uint8_t, 8> output;
		if (engine.Encrypt(vector.Plain.data(), 8, output.data()) != 8 || output != vector.Cipher) {
			std::puts("FAILED Blowfish encryption vector");
			failures++;
		}
		if (engine.Decrypt(vector.Cipher.data(), 8, output.data()) != 8 || output != vector.Plain) {
			std::puts("FAILED Blowfish decryption vector");
			failures++;
		}
		output = vector.Cipher;
		if (engine.Decrypt(output.data(), 8, output.data()) != 8 || output != vector.Plain) {
			std::puts("FAILED Blowfish in-place decryption vector");
			failures++;
		}
	}
	std::printf("Blowfish vectors: %d failures\n", failures);
	return(failures != 0);
}
