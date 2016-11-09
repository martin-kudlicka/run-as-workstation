// RunAsWorkstation.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdlib.h>
#include "runasworkstation.h"
#include <winerror.h>

int wmain()
{
  if (__argc < 2)
  {
    showHelp();
    return ERROR_BAD_ARGUMENTS;
  }

  return run();
}

