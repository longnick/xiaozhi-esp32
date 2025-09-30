#include "wallpaper_manager.h"
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>

static bool has_ext(const std::string& s, const char* ext){
  auto p = s.rfind('.');
  if (p==std::string::npos) return false;
  std::string e = s.substr(p+1);
  for (auto& c: e) c = tolower(c);
  return e==ext;
}

void WallpaperManager::Init(lv_obj_t* host, int disp_w, int disp_h){
  host_ = host;
  disp_w_ = disp_w; disp_h_ = disp_h;

  img_bg_ = lv_img_create(host_);
  lv_obj_set_size(img_bg_, disp_w_, disp_h_);
  lv_obj_align(img_bg_, LV_ALIGN_CENTER, 0, 0);
  lv_obj_clear_flag(img_bg_, LV_OBJ_FLAG_CLICKABLE);

  lbl_clock_ = lv_label_create(host_);
  lv_obj_set_style_text_font(lbl_clock_, lv_theme_get_font_large(host_), 0);
  lv_obj_set_style_text_color(lbl_clock_, lv_color_white(), 0);
  lv_obj_align(lbl_clock_, LV_ALIGN_TOP_MID, 0, 8);

  LoadList();
  PickRandom();
  ApplyToBg();
  UpdateClock();
  idle_ms_ = 0;
}

void WallpaperManager::OnUserActivity(){ idle_ms_ = 0; }

void WallpaperManager::Tick(uint32_t ms){
  idle_ms_ += ms;
  // update clock every second
  static uint32_t acc=0;
  acc += ms;
  if (acc >= 1000){
    acc = 0; UpdateClock();
  }
  // swap wallpaper every ~2 min (optional)
  if (idle_ms_ < 30000) return; // only clock while idle, screensaver handled elsewhere
}

void WallpaperManager::ForceNextWallpaper(){
  if (files_.empty()) return;
  idx_ = (idx_+1) % files_.size();
  ApplyToBg();
}

void WallpaperManager::LoadList(){
  files_.clear();
  DIR* d = opendir("/spiffs/wallpapers");
  if (!d) d = opendir("/wallpapers");
  if (!d) return;
  while (auto* e = readdir(d)){
    std::string n = e->d_name;
    if (n=="." || n=="..") continue;
    if (has_ext(n, "jpg") || has_ext(n, "jpeg") || has_ext(n, "png")){
      std::string full = std::string(e->d_name);
      // if not absolute, prefix with folder
      if (full[0] != '/'){
        full = std::string("/spiffs/wallpapers/") + full;
        struct stat st{};
        if (stat(full.c_str(), &st)!=0){
          full = std::string("/wallpapers/") + e->d_name;
        }
      }
      files_.push_back(full);
    }
  }
  closedir(d);
}

void WallpaperManager::PickRandom(){
  if (files_.empty()) return;
  uint32_t seed = lv_tick_get();
  idx_ = seed % files_.size();
}

static void fit_center_square(lv_obj_t* img_obj, int dw, int dh){
  // keep it simple: center image, let LVGL scale inside the square
  lv_obj_set_size(img_obj, dw, dh);
  lv_obj_align(img_obj, LV_ALIGN_CENTER, 0, 0);
}

bool WallpaperManager::LoadImgToLvgl(const char* path, lv_img_dsc_t* out){
  // Let LVGL handle decoding from file
  memset(out, 0, sizeof(*out));
  lv_img_set_src(img_bg_, path);
  return true;
}

void WallpaperManager::ApplyToBg(){
  if (idx_ < 0 || idx_ >= (int)files_.size()) return;
  const char* path = files_[idx_].c_str();
  lv_img_dsc_t dsc;
  if (LoadImgToLvgl(path, &dsc)){
    fit_center_square(img_bg_, disp_w_, disp_h_);
  }
}

void WallpaperManager::UpdateClock(){
  time_t now = time(nullptr);
  struct tm *tm_info = localtime(&now);
  char buf[16];
  strftime(buf, sizeof(buf), "%H:%M", tm_info);
  lv_label_set_text(lbl_clock_, buf);
}
