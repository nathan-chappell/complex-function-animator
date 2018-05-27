#include <gtkmm.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "parser.h"

using namespace std;

unique_ptr<EvalNodeBase> node;

C variable_z;
C slider_a;
C slider_b;
C slider_c;

double t = 0;
double grid_val = 10;
double step_grid = .5;

int resolution = 1000;

struct RGBA { double r,g,b,a; };
struct Point {double x, y;};
struct Line {
  Point l, r;
  RGBA color;
};

vector<Line> custom_line_buf;
Point cur_point;
bool push_line(false);

bool drag_in_progress = false;
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
    cout << cur_point.x << "..." << cur_point.y << endl;
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

  //for (double h = -1*grid_val; h <= grid_val; h += step_grid) ret.push_back(Line{{-1*grid_val,h},{grid_val,h}});
  return ret;
}

Point operator*(double s, Point p) { return {p.x*s, p.y*s}; }
Point operator-(Point l, Point r) { return {l.x - r.x, l.y - r.y};}
Point operator+(Point l, Point r) { return {l.x + r.x, l.y + r.y};}


vector<pair<vector<C>,RGBA>> transformed;

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
    variable_z = C(p.x,p.y);
    tr.push_back(node->Eval());
  }
  return make_pair(tr,ln.color);
}

void transform_lines()
{
  cout << "transforming lines" << endl;
  //cout << "res: " << resolution << endl;
  auto lines = get_grid();
  transformed.clear();
  for (auto&& ln : lines) transformed.push_back(transform_line(ln));
  for (auto&& ln : custom_line_buf) {
    cout << "transforming line..." << endl;
    transformed.push_back(transform_line(ln));
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

int main(int argc, char** argv) {

  Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv);

  Gtk::Window w = Gtk::Window();
  Gtk::ScrolledWindow sw = Gtk::ScrolledWindow();
  Gtk::Box box(Gtk::Orientation::ORIENTATION_VERTICAL);
  Gtk::Box hbox_1(Gtk::Orientation::ORIENTATION_HORIZONTAL);
  Gtk::Box hbox_2(Gtk::Orientation::ORIENTATION_HORIZONTAL);
  Gtk::Button map_b("map");
  Gtk::Label formula_l("formula");
  Gtk::Entry formula_e;
  formula_e.set_text("z^2");
  Gtk::Label resolution_l("res");
  Gtk::Entry resolution_e;
  resolution_e.set_text("1000");
  Gtk::Label grid_l("grid");
  Gtk::Entry grid_e;
  grid_e.set_text("10");
  Gtk::Label step_l("step");
  Gtk::Entry step_e;
  step_e.set_text("1");
  Gtk::Button clear("clear");
  Gtk::DrawingArea da;

  da.signal_draw().connect([&](const Cairo::RefPtr<Cairo::Context>& c){
      try {
	int res = stoi(resolution_e.get_text());
	double g_val = my_stod(grid_e.get_text());
	double s_grid = my_stod(step_e.get_text());
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
  da.show();

  auto set_eval_node = [&](){
    string text = formula_e.get_text();
    string foo;
    for (auto c : text) foo.push_back(c);
    cout << "text:" << endl;
    for (size_t i = 0; i < text.size(); ++i) cout << (int)text[i] << " ";
    cout << endl;
    cout << text << endl;
    node = unique_ptr<EvalNodeBase>(get_eval_tree(foo));
  };

  auto when_clicked = [&](){
    set_eval_node();
    transform_lines();
    sw.queue_draw();
  };
    
  clear.signal_clicked().connect([&](){ custom_line_buf.clear(); when_clicked(); });

  w.add(box);
  hbox_1.add(formula_l);
  hbox_1.add(formula_e);
  hbox_1.add(resolution_l);
  hbox_1.add(resolution_e);
  hbox_1.add(grid_l);
  hbox_1.add(grid_e);
  hbox_1.add(step_l);
  hbox_1.add(step_e);
  hbox_1.add(map_b);
  hbox_1.add(clear);


  w.set_events( Gdk::SCROLL_MASK);
  w.signal_scroll_event().connect([&](GdkEventScroll* e) {
      if (e->direction == GDK_SCROLL_UP) scale*=1.1;
      if (e->direction == GDK_SCROLL_DOWN) scale/=1.1;
      w.queue_draw();
      return false;
    });

   
  box.add(hbox_1);
  box.add(sw);

  map_b.signal_clicked().connect(when_clicked);
  formula_e.set_events( Gdk::KEY_PRESS_MASK );
  formula_e.signal_key_press_event().connect([&](GdkEventKey* e){
      if (e->keyval == GDK_KEY_Return) when_clicked();
      return false;
    });

  sw.add(da);
  sw.show_all();
  sw.set_policy(Gtk::PolicyType::POLICY_ALWAYS, Gtk::PolicyType::POLICY_ALWAYS);
  sw.set_min_content_width(800);
  sw.set_min_content_height(800);

  sw.set_events( Gdk::BUTTON_PRESS_MASK |
		 Gdk::BUTTON_RELEASE_MASK |
		 Gdk::BUTTON_MOTION_MASK 
		 );
  sw.signal_button_press_event().connect([&](GdkEventButton* e){
      cout << "but: " << e->button << endl;
      if (e->button == 1) {
	drag_in_progress = true;
	curx = e->x;
	cury = e->y;
	cout << "dragging in progress" << endl;
	cout << "curx: " << curx << endl;
	cout << "cury: " << cury << endl;
	cout << "tx: " << tx << endl;
	cout << "ty: " << ty << endl;
      }
      if (e->button == 3) {
	push_custom_point(e->x,e->y);
	when_clicked();
      }
      return false;
    });
  sw.signal_button_release_event().connect([&](GdkEventButton* e){
      if (e->button == 1) {
	drag_in_progress = false;
	//tx += e->x - curx;
	//ty += e->y - cury;
	cout << "no longer dragging" << endl;
	cout << "curx: " << curx << endl;
	cout << "cury: " << cury << endl;
	cout << "tx: " << tx << endl;
	cout << "ty: " << ty << endl;
      }
      return false;
    });
  sw.signal_motion_notify_event().connect([&](GdkEventMotion* e){
      if (drag_in_progress) {
	tx += e->x - curx;
	ty += e->y - cury;
	curx = e->x;
	cury = e->y;
	sw.queue_draw();
      }
      return false;
    });
      
  w.add_events( Gdk::KEY_PRESS_MASK );
  w.signal_key_press_event().connect([&](GdkEventKey* e) {
      cout << "keyval: " << e->keyval << endl;
      if (e->keyval == GDK_KEY_Escape) w.close();
      if (e->keyval == GDK_KEY_c) custom_line_buf.clear();
      return false;
    });
  //formula_e.add_events( Gdk::KEY_PRESS_MASK );
  formula_e.signal_activate().connect([&](){
      when_clicked();
      //return false;
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
  
  w.show_all();
  app->run(w);
}
