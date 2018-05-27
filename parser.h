//parser.h

#include <complex>
#include <string>

using C = std::complex<double>;

extern C variable_z;
extern C slider_a;
extern C slider_b;
extern C slider_c;

double my_stod(const std::string& str, size_t* p = nullptr);

class EvalNodeBase {
public:
  virtual C Eval() = 0;
  virtual int Priority() { return -1; }
  virtual EvalNodeBase** NextChild() { return nullptr; }
  EvalNodeBase* parent;
  virtual ~EvalNodeBase() {}
};

EvalNodeBase* get_eval_tree(const std::string& text);

