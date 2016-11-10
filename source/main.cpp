#include "stdafx.h"

#include "runasws.h"

int wmain()
{
  if (__argc < 2)
  {
    showHelp();
    return ERROR_BAD_ARGUMENTS;
  }

  return run();
}