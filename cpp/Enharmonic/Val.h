#pragma once

#include <vector>
#include <initializer_list>
#include "Ratio.h"
using namespace std;

class Val : public vector<Int>
{
public:
	Val(initializer_list<Int> args);
	Val();
};