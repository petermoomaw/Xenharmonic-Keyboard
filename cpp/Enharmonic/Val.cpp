#include "pch.h"
#include "Val.h"

Val::Val(initializer_list<Int> args)
{
	this->insert(this->end(), args.begin(), args.end());
}

Val::Val()
{
}