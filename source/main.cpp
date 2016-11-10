#include "stdafx.h"
#include <stdlib.h>
#include "runasws.h"
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