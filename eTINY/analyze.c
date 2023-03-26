/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include "globals.h"
#include "parse.h"
#include "util.h"

/* allocate global variables */
int lineno = 0;
FILE * source;
FILE * listing;
FILE * code;

/* allocate and set tracing flags */
int EchoSource = FALSE;
int TraceScan = FALSE;
int TraceParse = TRUE;

int Error = FALSE;

void AnalyzeCode()
{ TreeNode * syntaxTree;
  source = fopen("tmp_sourceCode.tmp", "r");
  listing = fopen("tmp_result.tmp", "w"); /* send listing to screen */
  fprintf(listing,"\nTINY COMPILATION:\n");
  syntaxTree = parse();
  if (TraceParse) {
    fprintf(listing,"\nSyntax tree:\n");
    printTree(syntaxTree);
  }
  fclose(source);
  fclose(listing);
}




