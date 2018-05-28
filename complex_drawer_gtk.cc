#include "complex_drawer_gtk.h"

#include <iostream>

using namespace std;

main_window::main_window() {
  set_events( Gdk::SCROLL_MASK |
	      Gdk::KEY_PRESS_MASK);

  signal_scroll_event().connect([&](GdkEventScroll* e) {
      if (e->direction == GDK_SCROLL_UP) scale*=1.1;
      if (e->direction == GDK_SCROLL_DOWN) scale/=1.1;
      queue_draw();
      return false;
    });

  add(opt_box);
  show_all();
}

s_window::s_window() {
  set_policy(Gtk::PolicyType::POLICY_ALWAYS, Gtk::PolicyType::POLICY_ALWAYS);
  set_min_content_width(1000);
  set_min_content_height(1000);

  set_events( Gdk::BUTTON_PRESS_MASK |
	      Gdk::BUTTON_RELEASE_MASK |
	      Gdk::BUTTON_MOTION_MASK 
	      );

  signal_button_release_event().connect([&](GdkEventButton* e){
      if (e->button == 1) {
	drag_in_progress = false;
      }
      return false;
    });
      
 
  add(da);
  show_all();
}

option_box::option_box() :
  Gtk::Box(Gtk::Orientation::ORIENTATION_VERTICAL),
  hbox_1(Gtk::Orientation::ORIENTATION_HORIZONTAL),
  hbox_2(Gtk::Orientation::ORIENTATION_HORIZONTAL),
  map_b("map"),
  animate_b("animate"),
  clear_b("clear"),
  formula_l("formula"),
  resolution_l("res"),
  grid_l("grid"),
  step_l("step")
{
  formula_e.set_text("z^2");
  resolution_e.set_text("1000");
  grid_e.set_text("10");
  step_e.set_text("1");


  hbox_1.add(formula_l);
  hbox_1.add(formula_e);
  hbox_1.add(resolution_l);
  hbox_1.add(resolution_e);
  hbox_1.add(grid_l);
  hbox_1.add(grid_e);
  hbox_1.add(step_l);
  hbox_1.add(step_e);
  hbox_1.add(map_b);
  hbox_1.add(clear_b);
  hbox_1.add(animate_b);

  map_b.signal_clicked().connect([&](){when_clicked();});

  formula_e.signal_key_press_event().connect([&](GdkEventKey* e){
      if (e->keyval == GDK_KEY_Return) when_clicked();
      return false;
    });

  formula_e.signal_activate().connect([&](){
      when_clicked();
    });
   
  resolution_e.signal_activate().connect([&](){
      when_clicked();
    });
  grid_e.signal_activate().connect([&](){
      when_clicked();
    });
  step_e.signal_activate().connect([&](){
      when_clicked();
    });
  
  add(hbox_1);
  add(hbox_2);

  formula_e.set_events( Gdk::KEY_PRESS_MASK );
}
