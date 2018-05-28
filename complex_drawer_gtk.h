//complex_drawer_gtk.h
#pragma once
#include <gtkmm.h>

class s_window;
class main_window;

extern double scale;
extern double curx;
extern double cury;
extern double tx;
extern double ty;
extern bool drag_in_progress;
extern s_window* sw_p;
extern main_window* w_p;

bool when_clicked();

class option_box : public Gtk::Box {
 public:
  Gtk::Box hbox_1;
  Gtk::Box hbox_2;

  Gtk::Button map_b;
  Gtk::Button animate_b;
  Gtk::Button clear_b;
  Gtk::Label formula_l;
  Gtk::Entry formula_e;
  Gtk::Label resolution_l;
  Gtk::Entry resolution_e;
  Gtk::Label grid_l;
  Gtk::Entry grid_e;
  Gtk::Label step_l;
  Gtk::Entry step_e;

  option_box();

};

class s_window : public Gtk::ScrolledWindow {
 public:
  Gtk::DrawingArea da;

  s_window();
};

class main_window : public Gtk::Window {
 public:

  option_box opt_box;

  main_window();
  
};
