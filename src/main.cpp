
#include "../imgui_impl_sdl3.h"
#include "../imgui_impl_sdlrenderer3.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <array>
#include <fcntl.h>
#include <spawn.h>
#include <stdexcept>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std;

extern char **environ;

vector<string> split_string(string str, const string &delimiter) {
  vector<string> strings;

  string::size_type pos = 0;
  string::size_type prev = 0;
  while ((pos = str.find(delimiter, prev)) != string::npos) {
    strings.push_back(str.substr(prev, pos - prev));
    prev = pos + delimiter.size();
  }

  strings.push_back(str.substr(prev));

  return strings;
}

void exec_th(string arg, string value) {
  pid_t pid;
  posix_spawn_file_actions_t file_actions;

  posix_spawn_file_actions_init(&file_actions);
  posix_spawn_file_actions_addopen(&file_actions, STDOUT_FILENO, "/dev/null",
                                   O_WRONLY, 0);
  posix_spawn_file_actions_addopen(&file_actions, STDERR_FILENO, "/dev/null",
                                   O_WRONLY, 0);

  const char *argv[] = {"hssc", arg.c_str(), value.c_str(), nullptr};

  posix_spawn(&pid, "/usr/bin/hssc", &file_actions, nullptr,
              (char *const *)argv, environ);

  posix_spawn_file_actions_destroy(&file_actions);
}

void exec(string arg, float value) {
  exec_th("--" + arg, to_string(value));
  this_thread::sleep_for(chrono::milliseconds(30));
}

string exec(const char *cmd) {
  array<char, 128> buffer;
  string result;
  unique_ptr<FILE, int (*)(FILE *)> pipe(popen(cmd, "r"), pclose);
  if (!pipe)
    throw runtime_error("popen() failed!");
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    result += buffer.data();
  return result;
}

float get_arg(vector<string> lines, string arg, float default_v = 1.0f) {
  for (auto line : lines) {
    vector<string> kv = split_string(line, "=");
    if (kv.size() != 2)
      continue;
    if (kv[0] == arg)
      return atof(kv[1].c_str());
  }
  return default_v;
}

void SetCustomImGuiStyle() {
  ImGuiStyle &style = ImGui::GetStyle();

  style.WindowRounding = 13.0f;
  style.FrameRounding = 7.3f;
  style.ScrollbarSize = 1.5f;
  style.GrabRounding = 2.3f;
  style.WindowBorderSize = 1.0f;
  style.FrameBorderSize = 1.0f;

  ImVec4 *colors = style.Colors;

  colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.105f, 0.11f, 1.0f);
  colors[ImGuiCol_Text] = ImVec4(0.86f, 0.93f, 0.89f, 1.0f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.0f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
}
// Main code
int main(int und_pos, char **) {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
    printf("Error: SDL_Init(): %s\n", SDL_GetError());
    return -1;
  }

  float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
  SDL_WindowFlags window_flags =
      SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
  SDL_Window *window = SDL_CreateWindow("HSSC GUI", (int)(537 * main_scale),
                                        (int)(337 * main_scale), window_flags);
  if (window == nullptr) {
    printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
    return -1;
  }
  SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
  SDL_SetRenderVSync(renderer, 1);
  if (renderer == nullptr) {
    SDL_Log("Error: SDL_CreateRenderer(): %s\n", SDL_GetError());
    return -1;
  }
  int pos = und_pos > 1 ? -1000 : SDL_WINDOWPOS_CENTERED;
  SDL_SetWindowPosition(window, pos, pos);
  SDL_ShowWindow(window);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  ImGui::StyleColorsDark();
  ImGuiStyle &style = ImGui::GetStyle();
  style.ScaleAllSizes(main_scale);
  style.FontScaleDpi = main_scale * 2;
  SetCustomImGuiStyle();
  ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer3_Init(renderer);

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  vector<string> lines = split_string(exec("hssc"), "\n");
  lines.erase(remove_if(lines.begin(), lines.end(),
                        [](string x) { return x.find('=') == string::npos; }),
              lines.end());

  static float temperature = get_arg(lines, "temperature", 6400.0f);
  static float brightness = get_arg(lines, "brightness");
  static float contrast = get_arg(lines, "contrast");
  static float gamma = get_arg(lines, "gamma");
  static float hue = get_arg(lines, "hue", 0.0f);
  static float saturation = get_arg(lines, "saturation");
  static float red = get_arg(lines, "red");
  static float green = get_arg(lines, "green");
  static float blue = get_arg(lines, "blue");

  // Main loop
  bool done = false;
  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (event.type == SDL_EVENT_QUIT)
        done = true;
      if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
          event.window.windowID == SDL_GetWindowID(window))
        done = true;
    }
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
      SDL_Delay(16);
      continue;
    }

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)width, (float)height));

    ImGui::Begin("HSSC GUI", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    {
      if (ImGui::DragFloat("temperature", &temperature, 50.0f, 1000.0f,
                           50000.0f, "%.1f")) {
        temperature = clamp(temperature, 1000.0f, 50000.0f);
        exec("temperature", temperature);
      }
      if (ImGui::DragFloat("brightness", &brightness, 0.001f, 0.1f, 5.0f,
                           "%.5f")) {
        brightness = clamp(brightness, 0.1f, 5.0f);
        exec("brightness", brightness);
      }
      if (ImGui::DragFloat("contrast", &contrast, 0.001f, 0.0f, 5.0f, "%.5f")) {
        contrast = clamp(contrast, 0.0f, 5.0f);
        exec("contrast", contrast);
      }
      if (ImGui::DragFloat("gamma", &gamma, 0.001f, 0.1f, 5.0f, "%.5f")) {
        gamma = clamp(gamma, 0.1f, 5.0f);
        exec("gamma", gamma);
      }
      if (ImGui::DragFloat("hue", &hue, 0.001f, -1.0f, 1.0f, "%.5f")) {
        hue = clamp(hue, -1.0f, 1.0f);
        exec("hue", hue);
      }
      if (ImGui::DragFloat("saturation", &saturation, 0.001f, 0.0f, 5.0f,
                           "%.5f")) {
        saturation = clamp(saturation, 0.0f, 5.0f);
        exec("saturation", saturation);
      }
      if (ImGui::DragFloat("red", &red, 0.001f, 0.0f, 5.0f, "%.5f")) {
        red = clamp(red, 0.0f, 5.0f);
        exec("red", red);
      }
      if (ImGui::DragFloat("green", &green, 0.001f, 0.0f, 5.0f, "%.5f")) {
        green = clamp(green, 0.0f, 5.0f);
        exec("green", green);
      }
      if (ImGui::DragFloat("blue", &blue, 0.001f, 0.0f, 5.0f, "%.5f")) {
        blue = clamp(blue, 0.0f, 5.0f);
        exec("blue", blue);
      }
    }
    ImGui::End();
    ImGui::Render();
    SDL_SetRenderScale(renderer, io.DisplayFramebufferScale.x,
                       io.DisplayFramebufferScale.y);
    SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y,
                                clear_color.z, clear_color.w);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
  }
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
