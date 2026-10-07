/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "surftext.h"

#include "dbgprint.h"
#include "surface.h"
#include "utf8.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>


struct SurfaceFont
{
	std::string Face;
	int Height;
	int Width;
	bool Bold;

	FT_Face Handle;
	// The horizontal stretch a requested average character width asks for.
	double ScaleX;
	int Ascent;
	int CellHeight;
};


namespace {

FT_Library Library = nullptr;
std::vector<std::unique_ptr<SurfaceFont>> Fonts;


// The macOS font folders, then the UI's own metric-compatible stand-in for Arial.
std::string Font_File(char const * face, bool bold)
{
	std::string const name = std::string(face) + (bold ? " Bold.ttf" : ".ttf");
	std::error_code error;
	char const * home = getenv("HOME");
	std::filesystem::path const directories[] = {
		"/System/Library/Fonts/Supplemental",
		"/System/Library/Fonts",
		"/Library/Fonts",
		std::filesystem::path(home != nullptr ? home : "") / "Library/Fonts",
	};
	for (std::filesystem::path const & directory : directories) {
		std::filesystem::path const path = directory / name;
		if (std::filesystem::is_regular_file(path, error)) {
			return(path.string());
		}
	}
	// The working directory is the executable's, inside an application bundle or not.
	for (char const * shipped : {"../Resources/ui/Arimo.ttf", "ui/Arimo.ttf"}) {
		if (std::filesystem::is_regular_file(shipped, error)) {
			return(shipped);
		}
	}
	return(std::string());
}


bool Open_Font(SurfaceFont & font)
{
	if (Library == nullptr && FT_Init_FreeType(&Library) != 0) {
		Library = nullptr;
		return(false);
	}

	std::string const path = Font_File(font.Face.c_str(), font.Bold);
	if (path.empty() || FT_New_Face(Library, path.c_str(), 0, &font.Handle) != 0) {
		DebugString("Text: no font file for %s%s\n", font.Face.c_str(), font.Bold ? " Bold" : "");
		return(false);
	}

	// Win32 measures a cell by the OS/2 table's Windows ascent and descent.
	FT_Face const face = font.Handle;
	TT_OS2 const * os2 = (TT_OS2 const *)FT_Get_Sfnt_Table(face, FT_SFNT_OS2);
	double const ascent = (os2 != nullptr && os2->usWinAscent != 0) ? os2->usWinAscent : face->ascender;
	double const descent = (os2 != nullptr && os2->usWinAscent != 0) ? os2->usWinDescent : -face->descender;
	double const units = face->units_per_EM;

	double const em = (font.Height < 0) ? -font.Height : font.Height * units / (ascent + descent);
	double const scale = em / units;
	FT_Set_Pixel_Sizes(face, 0, (FT_UInt)std::lround(em));

	font.Ascent = (int)std::lround(ascent * scale);
	font.CellHeight = (int)std::lround((ascent + descent) * scale);
	font.ScaleX = 1.0;
	if (font.Width != 0 && os2 != nullptr && os2->xAvgCharWidth > 0) {
		font.ScaleX = font.Width / (os2->xAvgCharWidth * scale);
	}

	FT_Matrix matrix = { (FT_Fixed)std::lround(font.ScaleX * 0x10000), 0, 0, 0x10000 };
	FT_Set_Transform(face, &matrix, nullptr);
	return(true);
}


unsigned short Blend_565(unsigned short pixel, int red, int green, int blue, int coverage)
{
	int const r = ((pixel >> 11) & 0x1F) * 255 / 31;
	int const g = ((pixel >> 5) & 0x3F) * 255 / 63;
	int const b = (pixel & 0x1F) * 255 / 31;
	int const outr = r + (red - r) * coverage / 255;
	int const outg = g + (green - g) * coverage / 255;
	int const outb = b + (blue - b) * coverage / 255;
	return((unsigned short)(((outr * 31 / 255) << 11) | ((outg * 63 / 255) << 5) | (outb * 31 / 255)));
}

}


SurfaceFont * Surface_Font(char const * face, int height, int width, bool bold)
{
	for (std::unique_ptr<SurfaceFont> const & font : Fonts) {
		if (font->Face == face && font->Height == height && font->Width == width && font->Bold == bold) {
			return(font.get());
		}
	}

	std::unique_ptr<SurfaceFont> font(new SurfaceFont{face, height, width, bold, nullptr, 1.0, 0, 0});
	if (height == 0 || !Open_Font(*font)) {
		return(nullptr);
	}
	Fonts.push_back(std::move(font));
	return(Fonts.back().get());
}


void Surface_Text_Extent(SurfaceFont * font, char const * text, int length, int & width, int & height)
{
	width = 0;
	height = (font != nullptr) ? font->CellHeight : 0;
	if (font == nullptr || text == nullptr) {
		return;
	}

	long advance = 0;
	char const * end = text + length;
	for (char const * cursor = text; cursor < end && *cursor != '\0';) {
		char32_t const code = UTF8::Decode(cursor);
		if (FT_Load_Char(font->Handle, code, FT_LOAD_DEFAULT) == 0) {
			advance += font->Handle->glyph->advance.x;
		}
	}
	width = (int)((advance + 32) >> 6);
}


int Surface_Draw_Text(Surface & surface, SurfaceFont * font, int x, int y, char const * text, int length, COLORREF color)
{
	if (font == nullptr || text == nullptr || surface.Bytes_Per_Pixel() != 2) {
		return(0);
	}

	unsigned short * const pixels = (unsigned short *)surface.Lock();
	if (pixels == nullptr) {
		return(0);
	}
	int const stride = surface.Stride() / 2;
	int const surfacewidth = surface.Get_Width();
	int const surfaceheight = surface.Get_Height();
	int const red = GetRValue(color);
	int const green = GetGValue(color);
	int const blue = GetBValue(color);

	long penx = (long)x << 6;
	int const baseline = y + font->Ascent;
	char const * end = text + length;
	for (char const * cursor = text; cursor < end && *cursor != '\0';) {
		char32_t const code = UTF8::Decode(cursor);
		if (FT_Load_Char(font->Handle, code, FT_LOAD_RENDER) != 0) {
			continue;
		}

		FT_GlyphSlot const glyph = font->Handle->glyph;
		FT_Bitmap const & bitmap = glyph->bitmap;
		int const left = (int)(penx >> 6) + glyph->bitmap_left;
		int const top = baseline - glyph->bitmap_top;
		for (unsigned row = 0; row < bitmap.rows; row++) {
			int const py = top + (int)row;
			if (py < 0 || py >= surfaceheight) {
				continue;
			}
			unsigned char const * coverage = bitmap.buffer + row * bitmap.pitch;
			for (unsigned column = 0; column < bitmap.width; column++) {
				int const px = left + (int)column;
				if (px >= 0 && px < surfacewidth && coverage[column] != 0) {
					unsigned short & pixel = pixels[py * stride + px];
					pixel = Blend_565(pixel, red, green, blue, coverage[column]);
				}
			}
		}
		penx += glyph->advance.x;
	}

	surface.Unlock();
	return((int)((penx - ((long)x << 6) + 32) >> 6));
}
