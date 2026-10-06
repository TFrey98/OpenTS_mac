// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2026 OpenTS contributors
// See LICENSE.md for applicable additional terms and warranty disclaimers.

#include "bgfxbackend.h"

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>

namespace {

int const FRAME_WIDTH = 640;
int const FRAME_HEIGHT = 480;

struct SDLSession {
	~SDLSession(void) { SDL_Quit(); }
};

struct RendererSession {
	~RendererSession(void) { Backend_Shutdown(); }
};


std::vector<std::uint16_t> Build_Frame(void)
{
	std::vector<std::uint16_t> pixels(FRAME_WIDTH * FRAME_HEIGHT);
	for (int y = 0; y < FRAME_HEIGHT; y++) {
		for (int x = 0; x < FRAME_WIDTH; x++) {
			unsigned red = x * 31 / (FRAME_WIDTH - 1);
			unsigned green = y * 63 / (FRAME_HEIGHT - 1);
			unsigned blue = ((x / 32 + y / 32) & 1) ? 31 : 0;
			pixels[y * FRAME_WIDTH + x] = (std::uint16_t)((red << 11) | (green << 5) | blue);
		}
	}
	return(pixels);
}

}


int main(int argc, char ** argv)
{
	bool smoke = false;
	for (int i = 1; i < argc; i++) {
		if (std::strcmp(argv[i], "--smoke-test") == 0) {
			smoke = true;
		} else if (std::strcmp(argv[i], "--help") == 0) {
			std::puts("OpenTS desktop macOS window and frame presenter\n"
				"  --smoke-test  Present 60 frames and exit\n"
				"  Escape       Close the window\n"
				"  Command+Return  Toggle desktop fullscreen");
			return(0);
		} else {
			std::fprintf(stderr, "Unknown argument: %s\n", argv[i]);
			return(1);
		}
	}

	SDL_SetMainReady();
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::fprintf(stderr, "SDL video initialization failed: %s\n", SDL_GetError());
		return(1);
	}
	SDLSession sdl;
	std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window(
		SDL_CreateWindow("OpenTS macOS - frame presenter", 960, 720,
			SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_METAL),
		SDL_DestroyWindow);
	if (!window) {
		std::fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
		return(1);
	}

	void * cocoa = SDL_GetPointerProperty(SDL_GetWindowProperties(window.get()),
		SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
	int width = 0;
	int height = 0;
	if (!cocoa || !SDL_GetWindowSizeInPixels(window.get(), &width, &height) || width <= 0 || height <= 0) {
		std::fprintf(stderr, "Cocoa window or drawable size is unavailable: %s\n", SDL_GetError());
		return(1);
	}

	NativeWindow native = {NATIVE_WINDOW_DEFAULT, nullptr, cocoa};
	if (!Backend_Init(native, width, height, BACKEND_RENDERER_AUTO, true)) {
		std::fputs("Engine frame presenter initialization failed\n", stderr);
		return(1);
	}
	RendererSession renderer;
	if (std::strcmp(Backend_Renderer_Name(), "Metal") != 0) {
		std::fprintf(stderr, "Expected Metal, received %s\n", Backend_Renderer_Name());
		return(1);
	}
	if (!Backend_Set_Frame_Size(FRAME_WIDTH, FRAME_HEIGHT)) {
		std::fputs("Engine frame texture creation failed\n", stderr);
		return(1);
	}
	std::printf("Desktop window: %s, renderer: %s, drawable: %d x %d\n",
		SDL_GetCurrentVideoDriver(), Backend_Renderer_Name(), width, height);
	std::fflush(stdout);

	std::vector<std::uint16_t> const pixels = Build_Frame();
	bool running = true;
	bool fullscreen = false;
	unsigned frames = 0;
	Uint64 const deadline = SDL_GetTicks() + 15000;
	while (running && (!smoke || frames < 60)) {
		if (smoke && SDL_GetTicks() > deadline) {
			std::fputs("Frame presenter smoke test timed out\n", stderr);
			return(1);
		}
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
				running = false;
			} else if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
				if (event.key.key == SDLK_ESCAPE) {
					running = false;
				} else if (event.key.key == SDLK_RETURN && (event.key.mod & SDL_KMOD_GUI) != 0) {
					if (SDL_SetWindowFullscreen(window.get(), !fullscreen)) {
						fullscreen = !fullscreen;
					}
				}
			} else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
				SDL_CaptureMouse(true);
			} else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP || event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
				SDL_CaptureMouse(false);
			}
		}
		if (!running) break;

		if ((SDL_GetWindowFlags(window.get()) & SDL_WINDOW_MINIMIZED) != 0 ||
			!SDL_GetWindowSizeInPixels(window.get(), &width, &height) || width <= 0 || height <= 0) {
			SDL_Delay(10);
			continue;
		}
		Backend_On_Resize(width, height);
		double scale = std::min((double)width / FRAME_WIDTH, (double)height / FRAME_HEIGHT);
		int framewidth = (int)(FRAME_WIDTH * scale);
		int frameheight = (int)(FRAME_HEIGHT * scale);
		if (!Backend_Present(pixels.data(), FRAME_WIDTH * 2, (width - framewidth) / 2,
			(height - frameheight) / 2, framewidth, frameheight, BACKEND_SCALE_NEAREST)) {
			std::fputs("Engine frame presentation failed\n", stderr);
			return(1);
		}
		Backend_End_Frame();
		frames++;
	}
	std::printf("Presented %u frames using %s\n", frames, Backend_Renderer_Name());
	return(smoke && frames != 60 ? 1 : 0);
}
