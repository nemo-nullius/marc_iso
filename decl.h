#include <map>
#include <string>
#include <string>
#include <map>
#include <iterator>
#include <iostream>
#include <vector>
#include <set>
#include <fstream>
#include <algorithm>
using std::cerr;
using std::clog;
using std::cout;
using std::endl;
using std::getline;
using std::ifstream;
using std::ios;
using std::make_pair;
using std::map;
using std::max;
using std::multimap;
using std::ofstream;
using std::pair;
using std::runtime_error;
using std::set;
using std::stoi;
using std::stoul;
using std::string;
using std::vector;

typedef multimap<string, map<string, string>> mmm;

#ifndef FUNC_WITHPARA
#define FUNC_WITHPARA
// default argument should only be defined in the function declaration
extern int stoi_safe(const std::string &str, int *p_value, std::size_t *pos = 0, int base = 10);
extern int stoi_safe(const std::string &str, size_t *p_value, std::size_t *pos = 0, int base = 10);
extern int stoi_safe(const std::string &&str, size_t *p_value, std::size_t *pos = 0, int base = 10);
#endif