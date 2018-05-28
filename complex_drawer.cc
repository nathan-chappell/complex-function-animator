#include <gtkmm.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "parser.h"
#include "complex_drawer_gtk.h"

using namespace std;

struct RGBA { double r,g,b,a; };
struct Point {double x, y;};
struct Line {
  Point l, r;
  RGBA color;
};

Point operator*(double s, Point p) { return {p.x*s, p.y*s}; }
Point operator-(Point l, Point r) { return {l.x - r.x, l.y - r.y};}
Point operator+(Point l, Point r) { return {l.x + r.x, l.y + r.y};}

s_window* sw_p;
main_window* w_p;

unique_ptr<EvalNodeBase> node;

double cur_time = 1;
double max_time = 3;
double time_delta = .05;

double grid_val = 10;
double step_grid = .5;

int resolution = 1000;

vector<Line> custom_line_buf;
Point cur_point;
bool push_line(false);

bool drag_in_progress = false;
bool smooth_draw = false;
bool smooth_draw_set_cur = false;
double curx;
double cury;
double tx = 400;
double ty = 400;
double scale = 10;

void push_custom_point(double x, double y) {
  if (!push_line) {
    cout << "storing point: " << x << ", " << y << endl;
    cur_point = Point{(x-tx)/scale,(y-ty)/scale};
  } else {
    //cout << cur_point.x << "..." << cur_point.y << endl;
    cout << "pushing point: "  << x << ", " << y << endl;
    custom_line_buf.push_back(Line{cur_point,Point{(x-tx)/scale,(y-ty)/scale},RGBA{1,0,1,1}});
  }
  push_line ^= true;
}

vector<Line> get_grid() {
  vector<Line> ret;
  //vertical:
  double color_delta = 1/((grid_val - -1*grid_val) / step_grid);
  for (double v = -1*grid_val, c = 0; v <= grid_val; v += step_grid, c += color_delta)
    ret.push_back(Line{{v,-1*grid_val},{v,grid_val},{0,c,c,.7}});
  //horizontal
  color_delta = 1/((grid_val - -1*grid_val) / step_grid);
  for (double h = -1*grid_val, c = 0; h <= grid_val; h += step_grid, c += color_delta)
    ret.push_back(Line{{-1*grid_val,h},{grid_val,h},{1,c,0,.7}});

  return ret;
}


vector<pair<vector<C>,RGBA>> transformed;
vector<vector<pair<vector<C>,RGBA>>> animate_Q;
size_t animate_index = 0;

Point resolve(const Line& ln, int step)
{
  double delta = step/(double)resolution;
  //cout << "delta: " << delta << endl;
  return ln.l + delta*(ln.r - ln.l);
}

pair<vector<C>,RGBA> transform_line(Line ln)
{
  vector<Point> t;
  for (int i = 0; i <= resolution; ++i) t.push_back(resolve(ln,i));
  vector<C> tr;
  for (auto&& p : t) {
    C cur = C(p.x,p.y);
    variable_z = cur;
    tr.push_back(cur_time*node->Eval() + (1-cur_time)*cur);
  }
  return make_pair(tr,ln.color);
}

void transform_lines()
{
  //cout << "transforming lines" << endl;
  //cout << "res: " << resolution << endl;
  auto lines = get_grid();
  transformed.clear();
  for (auto&& ln : lines) transformed.push_back(transform_line(ln));
  for (auto&& ln : custom_line_buf) {
    //cout << "transforming line..." << endl;
    transformed.push_back(transform_line(ln));
  }
}

void fill_animate_q()
{
  for (cur_time = 0; cur_time <= 1; cur_time += time_delta/max_time) {
    transform_lines();
    animate_Q.emplace_back(move(transformed));
  }
}

bool line_drawer(const Cairo::RefPtr<Cairo::Context>& c)
{
  for (auto&& vc : transformed) {
    c->move_to(vc.first.front().real(), vc.first.front().imag());
    c->set_source_rgba(vc.second.r, vc.second.g, vc.second.b, vc.second.a);
    for (auto&& cp : vc.first) {
      if (isnan(cp.real()) || isnan(cp.imag())) break;
      c->line_to(cp.real(), cp.imag());
    }
    c->stroke();
  }
  return false;
}

void draw_axis(const Cairo::RefPtr<Cairo::Context>& c)
{
  c->set_source_rgb(0,0,0);
  c->move_to(0,-1000);
  c->line_to(0,1000);
  c->stroke();
  c->move_to(-1000,0);
  c->line_to(1000,0);
  c->stroke();
  c->set_source_rgb(0,0,0);
}


void set_eval_node() {
  string text = w_p->opt_box.formula_e.get_text();
  string foo;
  for (auto c : text) foo.push_back(c);
  cout << "text:" << endl;
  for (size_t i = 0; i < text.size(); ++i) cout << (int)text[i] << " ";
  cout << endl;
  cout << text << endl;
  node = unique_ptr<EvalNodeBase>(get_eval_tree(foo));
};

bool when_clicked() {
  set_eval_node();
  transform_lines();
  sw_p->queue_draw();
  return false;
};

bool animate_next() {
  if (animate_index >= animate_Q.size()) return false;
  transformed = animate_Q[animate_index];
  ++animate_index;
  return true;
}

int main(int argc, char** argv) {

  Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv);

  s_window sw;
  main_window w;
  sw_p = &sw;
  w_p = &w;

  w.opt_box.hbox_2.add(sw);

  w.opt_box.clear_b.signal_clicked().connect([&](){
      custom_line_buf.clear();
      when_clicked();
    });

  sw.da.signal_draw().connect([&](const Cairo::RefPtr<Cairo::Context>& c){
      try {
	int res = stoi(w.opt_box.resolution_e.get_text());
	double g_val = my_stod(w.opt_box.grid_e.get_text());
	double s_grid = my_stod(w.opt_box.step_e.get_text());
	resolution = res;
	grid_val = g_val;
	step_grid = s_grid;
	//cout << "grid: " << grid_val << ", step: " << step_grid << endl;
      } catch (...) {}
      c->translate(tx,ty);
      c->scale(scale,scale);
      c->set_line_width(1/scale);
      draw_axis(c);
      c->set_line_width(2/scale);
      line_drawer(c);
      return false;
    });


  //box...


  w.signal_key_press_event().connect([&](GdkEventKey* e) {
      cout << "keyval: " << e->keyval << endl;
      if (e->keyval == GDK_KEY_Escape) w.close();
      else if (e->keyval == GDK_KEY_c) custom_line_buf.clear();
      else if (e->keyval == GDK_KEY_Control_L) {
	//cout << "shift: " << smooth_draw << endl;
	if (smooth_draw) smooth_draw = false;
	else {
	  smooth_draw = true;
	  smooth_draw_set_cur = true;
	}
      }
      return false;
    });


  sw.signal_button_press_event().connect([&](GdkEventButton* e){
      cout << "but: " << e->button << endl;
      if (e->button == 1) {
	drag_in_progress = true;
	curx = e->x;
	cury = e->y;
	//cout << "dragging in progress" << endl;
      }
      if (e->button == 3) {
	push_custom_point(e->x,e->y);
	when_clicked();
      }
      return false;
    });


  sw.signal_motion_notify_event().connect([&](GdkEventMotion* e){
      if (smooth_draw_set_cur) {
	smooth_draw_set_cur = false;
	curx = e->x;
	cury = e->y;
      }
      if (drag_in_progress) {
	tx += e->x - curx;
	ty += e->y - cury;
	curx = e->x;
	cury = e->y;
	sw.queue_draw();
      } else if (smooth_draw && hypot(e->x - curx, e->y - cury) > .1) {
	custom_line_buf.push_back(Line{
	    (1/scale)*Point{curx-tx,cury-ty},
	      (1/scale)*Point{e->x-tx, e->y-ty},RGBA{.8,.1,.9,.9}});
	curx = e->x;
	cury = e->y;
	when_clicked();
      }
      return false;
    });

  w.opt_box.animate_b.signal_clicked().connect([&]() {
      set_eval_node();
      animate_index = 0;
      animate_Q.clear();
      fill_animate_q();
      Glib::signal_timeout().connect([&]() {
	  w.queue_draw();
	  return animate_next();
	},
	50
	);});

  app->run(w);
}
