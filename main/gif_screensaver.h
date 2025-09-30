#pragma once
#include "lvgl.h"
#include <vector>
#include <string>

class GifScreensaver {
public:
  static void Init(lv_obj_t* host, int disp_w, int disp_h);
  static void Tick(uint32_t ms);              // call in the same tick as WallpaperManager
  static void OnUserActivity();               // reset idle
  static void ShowNow();                      // force-show (debug)

private:
  static void LoadList();
  static void StartGif(const char* path);
  static void StopGif();
  static void NextGif();

  static inline lv_obj_t* host_ = nullptr;
  static inline lv_obj_t* cont_ = nullptr;
  static inline lv_obj_t* gif_ = nullptr;
  static inline uint32_t idle_ms_ = 0;
  static inline uint32_t swap_acc_ = 0;
  static inline std::vector<std::string> files_;
  static inline int idx_ = -1;
  static inline int disp_w_ = 240, disp_h_ = 240;

  static constexpr uint32_t kIdleMsToStart = 30000;  // 30s
  static constexpr uint32_t kSwapEveryMs   = 5000;   // 5s
};
