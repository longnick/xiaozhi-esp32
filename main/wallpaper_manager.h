#pragma once
#include "lvgl.h"
#include <vector>
#include <string>

class WallpaperManager {
public:
  static void Init(lv_obj_t* host, int disp_w, int disp_h);
  static void Tick(uint32_t ms);              // call from your ui tick (e.g., every 50–100 ms)
  static void OnUserActivity();               // call on any touch/button/encoder
  static void ForceNextWallpaper();           // optional: manual next

private:
  static void LoadList();
  static void PickRandom();
  static void ApplyToBg();
  static void UpdateClock();
  static bool LoadImgToLvgl(const char* path, lv_img_dsc_t* out);

  static inline lv_obj_t* host_ = nullptr;
  static inline lv_obj_t* img_bg_ = nullptr;
  static inline lv_obj_t* lbl_clock_ = nullptr;
  static inline uint32_t idle_ms_ = 0;
  static inline int disp_w_ = 240, disp_h_ = 240;
  static inline std::vector<std::string> files_;
  static inline int idx_ = -1;
};
