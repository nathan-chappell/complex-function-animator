//The ugliest mother freaking parser in the world,
//by Nathan Chappell

#include "parser.h"

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <list>
#include <string>
#include <utility>

using namespace std;

enum class Function {
    kAbs, kArg, kCos, kExp, kLog, kSin, kTan, kReal,
    kImag, kNorm, kConj, kAcos, kAsin, kAtan, kCosh, kSinh,
    kSqrt, kTanh, kAcosh, kAsinh, kAtanh, kLog10
};

list<pair<string,Function>> function_list = {
  make_pair("acosh", Function::kAcosh),
  make_pair("asinh", Function::kAsinh),
  make_pair("atanh", Function::kAtanh),
  make_pair("log10", Function::kLog10),
  make_pair("real", Function::kReal),
  make_pair("imag", Function::kImag),
  make_pair("norm", Function::kNorm),
  make_pair("conj", Function::kConj),
  make_pair("acos", Function::kAcos),
  make_pair("asin", Function::kAsin),
  make_pair("atan", Function::kAtan),
  make_pair("cosh", Function::kCosh),
  make_pair("sinh", Function::kSinh),
  make_pair("sqrt", Function::kSqrt),
  make_pair("tanh", Function::kTanh),
  make_pair("abs", Function::kAbs),
  make_pair("arg", Function::kArg),
  make_pair("cos", Function::kCos),
  make_pair("exp", Function::kExp),
  make_pair("log", Function::kLog),
  make_pair("sin", Function::kSin),
  make_pair("tan", Function::kTan)
};

class Token {
public:
  enum Type { kParen, kFun, kVal, kOp, kI, kVar, kSlider, kEmpty, kEnd, kErr };
  Type type;
  string val;

  string PrettyPrint() const {
    string str = "<";
    switch (type) {
    case kParen: str += "kParen"; break;
    case kFun: str += "kFun"; break;
    case kVal: str += "kVal"; break;
    case kOp: str += "kOp"; break;
    case kI: str += "kI"; break;
    case kVar: str += "kVar"; break;
    case kSlider: str += "kSlider"; break;
    case kEmpty: str += "kEmpty"; break;
    case kEnd: str += "kEnd"; break;
    case kErr: str += "kErr"; break;
    }

    str += " : " + val + ">";
    return str;
  }
};

using cIt = list<Token>::const_iterator;
using It = list<Token>::iterator;

const size_t kErr = size_t(-1);
const Token kEmptyTok = {Token::kEmpty, ""};

Token ErrTok(const string& str) { return Token{Token::kErr, str}; }

bool token_valid(const Token& t)
{
  return (t.type != Token::kEmpty);
}

bool found_token(const pair<size_t, Token>& p)
{
  return (token_valid(p.second) && p.first != kErr);
}

pair<size_t, Token> try_to_read_function(const string& str, size_t pos)
{
  string sub;

  for (auto fp : function_list) {
    const string f = fp.first;
    size_t str_size = f.size();
    if (str.size() - pos < str_size) continue; //return make_pair(kErr, ErrTok("end of input"));
    sub = str.substr(pos, str_size);
    if (sub == f) return make_pair(pos + str_size, Token{Token::kFun, sub});
  }

  return make_pair(kErr, ErrTok("couldn't find function"));
}

size_t eat_ws(const string& str, size_t pos)
{
  while (pos < str.size() && isspace(str[pos])) ++pos;
  return pos;
}

/*
  I swear to god, the most inexplicable errors occur using the stod provided by the stl...
 */
double my_stod(const string& str, size_t* p)
{
  double d = 0;
  size_t count = 0;
  while (isdigit(str[count])) {
    d *= 10;
    d += str[count++] - '0';
  }
  if (str[count] == '.' && (count+1 < str.size()) && isdigit(str[count+1])) {
    ++count;
    double dec = 0;
    size_t dec_count = 0;
    while (isdigit(str[count])) {
      dec *= 10;
      dec += str[count++] - '0';
      ++dec_count;
    }
    for (size_t i = 0; i < dec_count; ++i) dec /= 10;
    d += dec;
  }
  if (count == 0) throw std::invalid_argument("not a double");
  if (p) *p = count;
  return d;
}

pair<size_t, Token> read_next(const string& str, size_t pos)
{
  pos = eat_ws(str, pos);
  cout << "pos: " << pos << endl;
  cout << "substr: " << str.substr(pos) << endl;

  if (pos == str.size()) return make_pair(pos, Token{Token::kEnd, ""});

  auto ftok = try_to_read_function(str, pos);
  if (found_token(ftok)) return ftok;

  if (str[pos] == '+') return make_pair(pos+1, Token{Token::kOp, "+"});
  if (str[pos] == '-') return make_pair(pos+1, Token{Token::kOp, "-"});

  try {
    size_t p = 0;
    string proc = str.substr(pos);
    cout << "processing: " << proc << endl;
    
    my_stod(proc, &p);
    cout << "double processed: " << p << endl;
    return make_pair(pos+p, Token{Token::kVal, str.substr(pos, p)});
  } catch (std::invalid_argument e) {
    cout << "not a double: " << str.substr(pos) << endl;
  }
 
  switch(str[pos]) {
  case 'z' : return make_pair(pos+1, Token{Token::kVar, "z"});
  case 'a' : return make_pair(pos+1, Token{Token::kSlider, "a"});
  case 'b' : return make_pair(pos+1, Token{Token::kSlider, "b"});
  case 'c' : return make_pair(pos+1, Token{Token::kSlider, "c"});
  case 'i' : return make_pair(pos+1, Token{Token::kI, "i"});
    //case '-' : return make_pair(pos+1, Token{Token::kOp, "-"});
    //case '+' : return make_pair(pos+1, Token{Token::kOp, "+"});
  case '*' : return make_pair(pos+1, Token{Token::kOp, "*"});
  case '/' : return make_pair(pos+1, Token{Token::kOp, "/"});
  case '^' : return make_pair(pos+1, Token{Token::kOp, "^"});
  case '(' : return make_pair(pos+1, Token{Token::kParen, "("});
  case ')' : return make_pair(pos+1, Token{Token::kParen, ")"});
  }

  return make_pair(kErr, ErrTok("couldn't match token"));
}

list<Token> Tokenize(const string& str)
{
  size_t pos = 0;
  list<Token> tokens;
  while (pos < str.size()) {
    auto p = read_next(str, pos);
    pos = p.first;
    tokens.push_back(p.second);
  }

  return tokens;
}

void dump_tokens(const list<Token>& tlist)
{
  for (auto&& t : tlist) cout << t.PrettyPrint() << endl;
}

/*
  1 | (), 
  2 | +
  3 | *, /
  4 | ^
  10000 | Head
 */

const int kMinPriority = INT32_MIN;

class ValueNode : public EvalNodeBase {
public:
  C val;
  ValueNode(C val) : val(val) {}
  C Eval() override { return val; }
  //C Eval() override {cout << "vnode: " << val << endl; return val; }
  ~ValueNode() {}
};


class SubExprNode : public EvalNodeBase {
public:
  EvalNodeBase* child;
  int Priority() override { return 1; }
  EvalNodeBase** NextChild() override { return &child; }
  C Eval() override { return child->Eval(); }
  //C Eval() override {cout << "subexprnode" << endl; return child->Eval(); }
  ~SubExprNode() { delete child; }
};

class VarNode : public EvalNodeBase {
public:
  EvalNodeBase* child;
  C* val;

  VarNode(string str) {
    if (str == "z") val = &variable_z;
    else if (str == "a") val = &slider_a;
    else if (str == "b") val = &slider_b;
    else if (str == "c") val = &slider_c;
    else assert(false && "couldn't create variable node!");
  }
  C Eval() override { return *val; }
  //C Eval() override { cout << "varnode: " << *val << endl; return *val; }
  ~VarNode() {}
};

class BinOperatorNode : public EvalNodeBase {
public:
  enum Operator { kPlus, kMult, kDiv, kPow };
  Operator op;
  EvalNodeBase *l, *r;
  
  BinOperatorNode(string str)
  {
    if (str == "+") op = kPlus;
    else if (str == "*") op = kMult;
    else if (str == "/") op = kDiv;
    else if (str == "^") op = kPow;
    else assert (false && "couldn't create BinOperatorNode!");
  }

  int Priority() override {
    switch(op) {
    case kPlus: return 2; break;
    case kMult: return 3; break;
    case kDiv: return 3; break;
    case kPow: return 4; break;
    default: assert (false && "couldn't determine priority BinOperatorNode!");
    }
  }

  EvalNodeBase** NextChild() override { return &r; }

  C Eval() override {
    /*
      cout << "binop ";
    
      switch(op) {
      case kPlus: cout << "kPlus" << endl; break;
      case kMult: cout << "kMult" << endl; break;
      case kDiv: cout << "kDiv" << endl; break;
      case kPow: cout << "kPow" << endl; break;
      default: assert (false && "couldn't eval BinOperatorNode!");
      }
    */
    switch(op) {
    case kPlus: return l->Eval() + r->Eval(); break;
    case kMult: return l->Eval() * r->Eval(); break;
    case kDiv: return l->Eval() / r->Eval(); break;
    case kPow: return pow(l->Eval(), r->Eval()); break;
    default: assert (false && "couldn't eval BinOperatorNode!");
    }
  }
      
  ~BinOperatorNode() { delete l; delete r; }
};

class FunctionNode : public EvalNodeBase {
public:
  Function f;
  EvalNodeBase* child;

  FunctionNode (string fn) {
    for (auto&& fp : function_list) {
      if (fn == fp.first) {
	f = fp.second;
	return;
      }
    }
    assert(false && "couldn't create function node!");
  }

  int Priority() { return 11; }
  EvalNodeBase** NextChild() override { return &child; }

  C Eval() override
  {
    //cout << "function" << endl;
    switch(f) {
    case Function::kAbs: return abs(child->Eval()); break;
    case Function::kArg: return arg(child->Eval()); break;
    case Function::kCos: return cos(child->Eval()); break;
    case Function::kExp: return exp(child->Eval()); break;
    case Function::kLog: return log(child->Eval()); break;
    case Function::kSin: return sin(child->Eval()); break;
    case Function::kTan: return tan(child->Eval()); break;
    case Function::kReal: return real(child->Eval()); break;
    case Function::kImag: return imag(child->Eval()); break;
    case Function::kNorm: return norm(child->Eval()); break;
    case Function::kConj: return conj(child->Eval()); break;
    case Function::kAcos: return acos(child->Eval()); break;
    case Function::kAsin: return asin(child->Eval()); break;
    case Function::kAtan: return atan(child->Eval()); break;
    case Function::kCosh: return cosh(child->Eval()); break;
    case Function::kSinh: return sinh(child->Eval()); break;
    case Function::kSqrt: return sqrt(child->Eval()); break;
    case Function::kTanh: return tanh(child->Eval()); break;
    case Function::kAcosh: return acosh(child->Eval()); break;
    case Function::kAsinh: return asinh(child->Eval()); break;
    case Function::kAtanh: return atanh(child->Eval()); break;
    case Function::kLog10: return log10(child->Eval()); break;
    default : assert(false && "couldn't evaluate function node!"); break;
    }
  }

  ~FunctionNode() { delete child; }
};

bool implied_multiplication_(It it1, It it2) {
  return ((it1->type == Token::kSlider && it2->type == Token::kSlider) ||
	  (it1->type == Token::kSlider && it2->type == Token::kVal) ||
	  (it1->type == Token::kSlider && it2->type == Token::kVar)||
	  (it1->type == Token::kSlider && it2->type == Token::kI)||
	  (it1->type == Token::kVal && it2->type == Token::kVal) ||
	  (it1->type == Token::kVal && it2->type == Token::kVar)||
	  (it1->type == Token::kVal && it2->type == Token::kI)||
	  (it1->type == Token::kVar && it2->type == Token::kVar)||
	  (it1->type == Token::kVar && it2->type == Token::kI)||
	  (it1->type == Token::kI && it2->type == Token::kI)
	  );
}

bool implied_multiplication(It it) {
  return (implied_multiplication_(it, next(it,1)) ||
	  implied_multiplication_(next(it,1), it));
}

//eliminate - operators and make implied multiplication explicit
void adjust_token_list(list<Token>& tlist)
{
  for (auto&& it = tlist.begin(); it != prev(tlist.end(),1); ++it) {
    if (it->type == Token::kOp && it->val == "-") {
      it++->val = "+";
      tlist.insert(it, Token{Token::kVal, "-1"});
      tlist.insert(it, Token{Token::kOp, "*"});
    }
    else if (implied_multiplication(it)) {
      ++it;
      tlist.insert(it, Token{Token::kOp, "*"});
    }
  }
}

#define LINKINTERNAL next->parent = prev; prev = next; *cur = next;
#define LINKLEAF next->parent = prev; *cur = next;
//enum Type { kParen, kFun, kVal, kI, kOp, kVar, kSlider, kEmpty, kEnd, kErr };
EvalNodeBase* get_eval_tree(const list<Token>& tlist)
{
  //HeadNode* head = new HeadNode();
  SubExprNode* head = new SubExprNode();
  head->parent = nullptr;
  EvalNodeBase* prev = head;
  EvalNodeBase** cur = &head->child;
  //cur = &head->child;

  for (auto&& curTok = tlist.begin(); curTok != tlist.end(); ++curTok) {
    switch (curTok->type) {
    case Token::kParen: {
      if (curTok->val == "(") {
	SubExprNode* next = new SubExprNode();
	LINKINTERNAL;
	cur = &next->child;
      } else {
	while (prev->Priority() != 1) prev = prev->parent;
	if (prev != head) prev = prev->parent;
	assert(prev && "error finding closing paren!");
	cur = prev->NextChild();
	assert(cur && "error closing paren!");
      }
    } break;
    case Token::kFun: {
      FunctionNode* next = new FunctionNode(curTok->val);
      LINKINTERNAL;
      cur = &next->child;
    } break;
    case Token::kVal: {
      ValueNode* next = new ValueNode(my_stod(curTok->val));
      LINKLEAF;
    } break;
    case Token::kI: {
      ValueNode* next = new ValueNode(C(0,1));
      LINKLEAF;
    } break;
    case Token::kOp: {
      BinOperatorNode* next = new BinOperatorNode(curTok->val);
      while (prev->Priority() > next->Priority() && prev != head) {
	prev = prev->parent;
	assert (prev && "error climbing tree! - prev");
      }
      cur = prev->NextChild();
      assert (cur && "error climbing tree! cur");

      next->parent = prev;
      next->l = *cur;
      (*cur)->parent = next;
      prev = next;
      *cur = next;
      cur = &next->r;
    } break;
    case Token::kVar: {
      VarNode* next = new VarNode(curTok->val);
      LINKLEAF;
    } break;
    case Token::kSlider: {
      VarNode* next = new VarNode(curTok->val);
      LINKLEAF;
    } break;
    case Token::kEmpty: break;
    case Token::kEnd: {
      return head;
    } break;
    case Token::kErr: {
      cout << string("error occured at: " + curTok->val) << endl;
      assert(false );
    } break;
    }
  }

  return head;
}

EvalNodeBase* get_eval_tree(const string& text)
{
  auto tlist = Tokenize(text);
  dump_tokens(tlist);
  adjust_token_list(tlist);
  dump_tokens(tlist);
  return get_eval_tree(tlist);
}

/*
#include <iostream>
C variable_z;
C slider_a;
C slider_b;
C slider_c;

int main(int argc, char** argv) {
  string text;
  if (argc > 1) text = argv[1];
  else return 0;

  variable_z = C(1,1);
  slider_a = C(2,1);
  slider_b = C(1,2);
  slider_c = C(7,11);

  auto tlist = Tokenize(text);
  adjust_token_list(tlist);
  dump_tokens(tlist);
  EvalNodeBase* node = get_eval_tree(tlist);
  cout << node->Eval() << endl;
}
*/
/*
Function list:

abs
arg
cos
exp
log
sin
tan
real
imag
norm
conj
acos
asin
atan
cosh
sinh
sqrt
tanh
acosh
asinh
atanh
log10

*/ 
