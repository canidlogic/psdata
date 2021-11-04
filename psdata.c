/*
 * psdata.c
 * ========
 * 
 * See README.md for further information.
 */

/*
 * Detect whether we are being compiled on Windows.
 * 
 * The predefined macros we check for here are from the project
 * "Pre-defined Compiler Macros" in the "Operating systems" section:
 * 
 * https://sourceforge.net/p/predef/wiki/Home/
 */
#ifdef _WIN32
#define PSDATA_WIN
#endif

#ifdef _WIN64
#define PSDATA_WIN
#endif

#ifdef __WIN32__
#define PSDATA_WIN
#endif

#ifdef __TOS_WIN__
#define PSDATA_WIN
#endif

#ifdef __WINDOWS__
#define PSDATA_WIN
#endif

/*
 * Include core headers.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Windows-only additional headers.
 */
#ifdef PSDATA_WIN
#include <fcntl.h>
#include <io.h>
#endif

/*
 * Constants
 * =========
 */

/*
 * The maximum line length allowed by PostScript Document Structuring
 * Conventions, not including the line break.
 */
#define MAX_PSLINE (255)

/*
 * The default line length to use if none is explicitly given.
 */
#define DEFAULT_LINE (72)

/*
 * The minimum valid line length that can be set.
 */
#define MIN_LINE (16)

/*
 * Local data
 * ==========
 */

/*
 * The name of the executing module, for use in error messages.
 * 
 * This is set at the start of the program entrypoint.
 */
const char *pModule = NULL;

/*
 * Local functions
 * ===============
 */

/* Prototypes */
static int check_head(const char *pstr);
static int parseInt(const char *pstr, int32_t *pv);

/*
 * Check whether the given parameter is a valid header string.
 * 
 * In order to pass the check, the given string must:
 * 
 *   1) Have no more than MAX_PSLINE characters.
 *   2) Contain only characters in range [0x20, 0x7e].
 * 
 * A fault occurs if NULL is passed.
 * 
 * Parameters:
 * 
 *   pstr - the header string to check
 * 
 * Return:
 * 
 *   non-zero if header is OK, zero if not
 */
static int check_head(const char *pstr) {
  
  int result = 1;
  
  /* Check parameter */
  if (pstr == NULL) {
    abort();
  }
  
  /* Verify length constraint */
  if (strlen(pstr) > MAX_PSLINE) {
    result = 0;
  }
  
  /* Verify character constraint */
  if (result) {
    for( ; *pstr != 0; pstr++) {
      if ((*pstr < 0x20) || (*pstr > 0x7e)) {
        result = 0;
        break;
      }
    }
  }
  
  /* Return result */
  return result;
}

/*
 * Parse the given string as a signed integer.
 * 
 * pstr is the string to parse.
 * 
 * pv points to the integer value to use to return the parsed numeric
 * value if the function is successful.
 * 
 * In two's complement, this function will not successfully parse the
 * least negative value.
 * 
 * Parameters:
 * 
 *   pstr - the string to parse
 * 
 *   pv - pointer to the return numeric value
 * 
 * Return:
 * 
 *   non-zero if successful, zero if failure
 */
static int parseInt(const char *pstr, int32_t *pv) {
  
  int negflag = 0;
  int32_t result = 0;
  int status = 1;
  int32_t d = 0;
  
  /* Check parameters */
  if ((pstr == NULL) || (pv == NULL)) {
    abort();
  }
  
  /* If first character is a sign character, set negflag appropriately
   * and skip it */
  if (*pstr == '+') {
    negflag = 0;
    pstr++;
  } else if (*pstr == '-') {
    negflag = 1;
    pstr++;
  } else {
    negflag = 0;
  }
  
  /* Make sure we have at least one digit */
  if (*pstr == 0) {
    status = 0;
  }
  
  /* Parse all digits */
  if (status) {
    for( ; *pstr != 0; pstr++) {
    
      /* Make sure in range of digits */
      if ((*pstr < '0') || (*pstr > '9')) {
        status = 0;
      }
    
      /* Get numeric value of digit */
      if (status) {
        d = (int32_t) (*pstr - '0');
      }
      
      /* Multiply result by 10, watching for overflow */
      if (status) {
        if (result <= INT32_MAX / 10) {
          result = result * 10;
        } else {
          status = 0; /* overflow */
        }
      }
      
      /* Add in digit value, watching for overflow */
      if (status) {
        if (result <= INT32_MAX - d) {
          result = result + d;
        } else {
          status = 0; /* overflow */
        }
      }
    
      /* Leave loop if error */
      if (!status) {
        break;
      }
    }
  }
  
  /* Invert result if negative mode */
  if (status && negflag) {
    result = -(result);
  }
  
  /* Write result if successful */
  if (status) {
    *pv = result;
  }
  
  /* Return status */
  return status;
}

/*
 * Program entrypoint
 * ==================
 */

int main(int argc, char *argv[]) {
  
  int status = 1;
  int i = 0;
  
  int32_t line_len = DEFAULT_LINE;
  int flag_dsc = 0;
  const char *pHead = NULL;
  
  /* Get program name */
  pModule = NULL;
  if ((argc > 0) && (argv != NULL)) {
    pModule = argv[0];
  }
  if (pModule == NULL) {
    pModule = "psdata";
  }
  
  /* On Windows only, set input mode to binary and output mode to
   * text */
#ifdef PSDATA_WIN
  if (status) {
    if (_setmode(_fileno(stdin), _O_BINARY) == -1) {
      status = 0;
      fprintf(stderr, "%s: Failed to set input to binary mode!\n",
        pModule);
    }
  }
  if (status) {
    if (_setmode(_fileno(stdout), _O_TEXT) == -1) {
      status = 0;
      fprintf(stderr, "%s: Failed to set output to text mode!\n",
        pModule);
    }
  }
#endif
  
  /* Check that any passed parameters are present */
  if (status && (argc > 0)) {
    if (argv == NULL) {
      abort();
    }
    for(i = 0; i < argc; i++) {
      if (argv[i] == NULL) {
        abort();
      }
    }
  }
  
  /* Interpret any extra parameters beyond the program name that were
   * passed */
  if (status) {
    for(i = 1; i < argc; i++) {
      /* Determine current option */
      if (strcmp(argv[i], "-dsc") == 0) {
        /* Set Document Structuring Conventions mode flag */
        flag_dsc = 1;
        
      } else if (strcmp(argv[i], "-head") == 0) {
        /* Header option requires an additional parameter */
        if (i >= argc - 1) {
          status = 0;
          fprintf(stderr, "%s: -head option requires a parameter!\n",
            pModule);
        }
        
        /* We will also consume the next parameter */
        if (status) {
          i++;
        }
        
        /* Check that parameter is valid header line */
        if (status) {
          if (!check_head(argv[i])) {
            status = 0;
            fprintf(stderr, "%s: -head option value is not valid!\n",
              pModule);
          }
        }
        
        /* Store the header line */
        if (status) {
          pHead = argv[i];
        }
        
      } else if (strcmp(argv[i], "-len") == 0) {
        /* Length option requires an additional parameter */
        if (i >= argc - 1) {
          status = 0;
          fprintf(stderr, "%s: -len option requires a parameter!\n",
            pModule);
        }
        
        /* We will also consume the next parameter */
        if (status) {
          i++;
        }
        
        /* Set the line length */
        if (status) {
          if (!parseInt(argv[i], &line_len)) {
            status = 0;
            fprintf(stderr, "%s: -len option value is not valid!\n",
              pModule);
          }
        }
        
        /* Check the line length setting */
        if (status) {
          if ((line_len < MIN_LINE) || (line_len > MAX_PSLINE)) {
            status = 0;
            fprintf(stderr, "%s: -len option value out of range!\n",
              pModule);
          }
        }
        
      } else {
        /* Unrecognized option */
        status = 0;
        fprintf(stderr, "%s: Unrecognized option: %s\n",
          pModule, argv[i]);
      }
      
      /* Leave loop if error */
      if (!status) {
        break;
      }
    }
  }
  
  /* If a header line was given, check that it does not exceed the line
   * length setting */
  if (status && (pHead != NULL)) {
    if (strlen(pHead) > line_len) {
      status = 0;
      fprintf(stderr, "%s: Header line is longer than line length!\n",
        pModule);
    }
  }
  
  /* @@TODO: */
  
  /* Invert status and return */
  if (status) {
    status = 0;
  } else {
    status = 1;
  }
  return status;
}