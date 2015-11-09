#ifndef __ROBOT_H__
#define __ROBOT_H__

#include "Singleton.h"

class Robot : public Singleton<Robot>
{
	SINGLETON(Robot);
};

#endif