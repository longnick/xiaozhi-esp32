#include "gif_screensaver.h"
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

void GifScreensaver::Init(lv_obj_t* host, int disp_w, int disp_h){
  host_ = host;
  disp_w_ = disp_w; disp_h_ = disp_h;

  cont_ = lv_obj_create(host_);
  lv_obj_set_size(cont_, disp_w_, disp_h_);
  lv_obj_align(cont_, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_flag(cont_, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(cont_, LV_OBJ_FLAG_SCROLLABLE);

  gif_ = lv_gif_create(cont_);
  lv_obj_set_size(gif_, disp_w_, disp_h_);
  lv_obj_align(gif_, LV_ALIGN_CENTER, 0, 0);

  LoadList();
  idle_ms_ = 0;
  swap_acc_ = 0;
}

void GifScreensaver::OnUserActivity(){
  idle_ms_ = 0;
  swap_acc_ = 0;
  StopGif();
}

void GifScreensaver::Tick(uint32_t ms){
  idle_ms_ += ms;
  if (idle_ms_ < kIdleMsToStart) return;

  if (lv_obj_has_flag(cont_, LV_OBJ_FLAG_HIDDEN)){
    // first time show
    NextGif();
    lv_obj_clear_flag(cont_, LV_OBJ_FLAG_HIDDEN);
  }

  swap_acc_ += ms;
  if (swap_acc_ >= kSwapEveryMs){
    swap_acc_ = 0;
    NextGif();
  }
}

void GifScreensaver::ShowNow(){
  idle_ms_ = kIdleMsToStart;
  Tick(0);
}

void GifScreensaver::LoadList(){
  files_.clear();
  DIR* d = opendir("/spiffs/mochi");
  if (!d) d = opendir("/mochi");
  if (!d) return;
  while (auto* e = readdir(d)){
    std::string n = e->d_name;
    if (n=="." || n=="..") continue;
    if (has_ext(n, "gif")){
      std::string full = std::string(e->d_name);
      if (full[0] != '/'){
        full = std::string("/spiffs/mochi/") + full;
        struct stat st{};
        if (stat(full.c_str(), &st)!=0){
          full = std::string("/mochi/") + e->d_name;
        }
      }
      files_.push_back(full);
    }
  }
  closedir(d);
}

void GifScreensaver::StartGif(const char* path){
  lv_gif_set_src(gif_, path);   // uses lv_gif file decoder if built
}

void GifScreensaver::StopGif(){
  if (!lv_obj_has_flag(cont_, LV_OBJ_FLAG_HIDDEN)){
    lv_obj_add_flag(cont_, LV_OBJ_FLAG_HIDDEN);
  }
}

void GifScreensaver::NextGif(){
  if (files_.empty()) return;
  uint32_t seed = lv_tick_get();
  idx_ = (seed + idx_ + 1) % files_.size();
  StartGif(files_[idx_].c_str());
}
