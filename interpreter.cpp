#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <conio.h>

#define INPUT_FILENAME "code_file.gcc"
//#define INPUT_FILENAME "input.txt"
#define OUTPUT_FILENAME "code.art"

#define SAFE_RELEASE
#define TAKE_INPUT_FROM_TEXT_FILE
#define WRITE_OUTPUT_TO_TEXT_FILE

using namespace std;

typedef enum
{
        TOO_MANY_TOKENS_ERROR = 0,
        TOO_LONG_INSTRUCTION_ERROR,
        UNIDENTIFIED_TOKEN,
        MISSING_SINGLE_QUOTE,
        MISSING_DOUBLE_QUOTE,
        MISSING_SEMI_COLON,
        EXCESS_STATEMENTS
};

typedef enum
{
        OPERATIONAL_TOKEN = 0,
        RELATIONAL_OPERAND_TOKEN,
        SINGLE_QUOTATION_TOKEN,
        DOUBLE_QUOTATION_TOKEN,
        SEMICOLON_TOKEN,
        COLON_TOKEN,
        GROUP_TOKEN
};

FILE *pFile;

typedef char String[1001]; 
/* Strings are 1000 length at maximum, but the convention is that it will be scanned until the 256th character*/

/* Global variables */
   String STR_NEXT_LINE;
   long Line_Number;
/* End global variables */

String Queue[1000];
String Symbol[100];
String Stack[1000];

String Bracket_Stack;


int QUEUE_INDEX;
int STACK_INDEX;
int SYMBOL_INDEX;
int BRACKET_INDEX;
bool HAS_ERROR;

/************* Variable storage ***************/

typedef enum
{
        INT = 0,
        CHAR,
        FLOAT,
        DOUBLE,
        STRING,
        BOOLEAN
};

typedef struct
{
        String var_name;
        int var_type;
        String var_value;
} VARIABLE;

VARIABLE VARIABLES[1000];

/************* Variable storage ***************/

void PRINT_STACK_CONTENTS()
{
     for (int i = 0;i < STACK_INDEX;i++)
         fprintf(pFile,"%s ",Stack[i]);
     fprintf (pFile,"\n");
}

void ENQUEUE(String CAND_STRING,String CAND_STRING_2)
{
     strcpy(Queue[QUEUE_INDEX],CAND_STRING);
     QUEUE_INDEX++;
     strcpy(Symbol[SYMBOL_INDEX],CAND_STRING_2);
     SYMBOL_INDEX++;
}

void DEQUEUE(String CAND_STRING)
{
     strcpy(CAND_STRING,Queue[QUEUE_INDEX-1]);
     strcpy(Queue[QUEUE_INDEX-1],"");
     QUEUE_INDEX--;
}

void PUSH(String CAND_STRING)
{
     for (int i = 0;i < QUEUE_INDEX;i++)
     {
   //      fprintf (pFile,"%s == %s\n",Queue[i],CAND_STRING);
         if (strcmp(Queue[i],CAND_STRING) == 0)
         {
    //        fprintf (pFile,"TRUE\n");
            strcpy(Stack[STACK_INDEX],Symbol[i]);
            STACK_INDEX++;
            return;
         }
     }
     
     strcpy(Stack[STACK_INDEX],"IDENTIFIER");
     STACK_INDEX++;
}

void POP(String CAND_STRING)
{
     if (STACK_INDEX <= 0)
     {
         strcpy(CAND_STRING,"");
         return;
     }
     strcpy(CAND_STRING,Stack[STACK_INDEX-1]);
     STACK_INDEX--;
}

void PUSH_BRACKET()
{
     Bracket_Stack[BRACKET_INDEX] = '{';
     BRACKET_INDEX++;
}

bool POP_BRACKET()
{
     if (BRACKET_INDEX <= 0)
         return false;
     else
     {
         BRACKET_INDEX--;
         return true;
     }
}

void PEEK(String CAND_STRING)
{
     strcpy(CAND_STRING,Stack[STACK_INDEX-1]);
}

#ifdef SAFE_RELEASE
void INIT_MAIN_PROGRAM_CONTENTS()
{
     memset(STR_NEXT_LINE,'\0',sizeof(String));
     Line_Number = 0;
     QUEUE_INDEX = 0;
     STACK_INDEX = 0;
     SYMBOL_INDEX = 0;
     BRACKET_INDEX = 0;
     HAS_ERROR = false;
     
     ENQUEUE("main","main_optr");
     ENQUEUE("int","int_optr");
     ENQUEUE("char","char_optr");
     ENQUEUE("float","float_optr");
     ENQUEUE("string","string_optr");
     ENQUEUE("boolean","boolean_optr");
     ENQUEUE("void","void_optr");
     ENQUEUE("if","if_optr");
   //  ENQUEUE("else","else_optr");
     ENQUEUE("return","return_optr");
  //   ENQUEUE("for","for_optr");
     ENQUEUE("test","test_optr");
     ENQUEUE("then","then_optr");
     ENQUEUE("set","set_optr");
     ENQUEUE("while","while_optr");
     ENQUEUE("read","read_optr");
     ENQUEUE("write","write_optr");
     ENQUEUE("(","op_paren");
     ENQUEUE(")","cl_paren");
     ENQUEUE("[","op_brack");
     ENQUEUE("]","cl_brack");
     ENQUEUE("{","op_brace");
     ENQUEUE("}","op_brace");
     ENQUEUE(";","sem_col");
     ENQUEUE("+","optr");
     ENQUEUE("-","optr");
     ENQUEUE("*","optr");
     ENQUEUE("/","optr");
     ENQUEUE(">","rel_optr");
     ENQUEUE("<","rel_optr");
     ENQUEUE(">=","rel_optr");
     ENQUEUE("<=","rel_optr");
     ENQUEUE("==","rel_optr");
     ENQUEUE("=","ass_val");
     ENQUEUE("\"","doub_quote");
     ENQUEUE("\'","sing_quote");
     ENQUEUE("|","comp_log_or");
     ENQUEUE("&","comp_log_amp");
     ENQUEUE(",","comma");
     ENQUEUE("#","number_sign");
     ENQUEUE("test","test_compare");
     ENQUEUE("then","then_compare");
     ENQUEUE("true","bool_sign");
     ENQUEUE("false","bool_sign");
     ENQUEUE("INTEGER","INTEGER");
     ENQUEUE("FLOATING","FLOATING");
     ENQUEUE("function","func");
     ENQUEUE("as","as");
}
#endif

void CLEAR_STACK()
{
     STACK_INDEX = 0;
}

bool IS_ALPHA (char CHARACTER)
{
     if (CHARACTER >= 'a' && CHARACTER <= 'z' || CHARACTER >= 'A' && CHARACTER <= 'Z' || CHARACTER == '_')
        return true;
     return false;
}

bool IS_DIGIT (char CHARACTER)
{
     if (CHARACTER >= '0' && CHARACTER <= '9')
        return true;
     return false;
}

bool IS_SINGLE_QUOTE(char CHARACTER)
{
     if (CHARACTER == '\'')
        return true;
     return false;
}

bool IS_DOUBLE_QUOTE(char CHARACTER)
{
     if (CHARACTER == '"')
        return true;
     return false;
}

bool IS_DOT_CHARACTER(char CHARACTER)
{
     if (CHARACTER == '.')
        return true;
     return false;
}

/* This only Prints out the line number */

void DRAW_LINE_NUMBER()
{
     fprintf(pFile,"On Line %d:\n",Line_Number);
}

/* This just prints a new line */

void DRAW_NEW_LINE()
{
     fprintf(pFile,"\n");
}

/* Given a single character as a parameter, it will return the first index of the 
character passed as a parameter. It also accepts the length of the string as the
parameter, which is actually the end index of the string's section. It will return
-1 if the character is not found.*/

int GET_FIRST_INDEX(char TARGET_CHAR,int FINAL_INDEX)
{
    for (int i = 0;i < FINAL_INDEX;i++)
        if (STR_NEXT_LINE[i] == TARGET_CHAR)
           return i;
    
    return -1;
}

/* This is just like the GET_FIRST_INDEX, but it returns the index of the 2nd instance
of the character. This is used for scanning ' characters and " and finding if it is
terminated or unterminted */

int GET_SECOND_INDEX(char TARGET_CHAR,int FINAL_INDEX)
{
    int FIRST_INSTANCE = GET_FIRST_INDEX(TARGET_CHAR,FINAL_INDEX);
    /* Not found? Return -1 */
    if (FIRST_INSTANCE > -1)
    {
       for (int i = FIRST_INSTANCE + 1;i < FINAL_INDEX;i++)
       {
           /* The moment the second instance of the character is found, we immediately return the index */
           if (STR_NEXT_LINE[i] == TARGET_CHAR)
              return i;
       }
       /*Otherwise, we return -1, meaning that it has no second index*/
       return -1;
    }
    /* Otherwise, we return FIRST_INSTANCE, which is presumably -1 if it does not enter the if statement above */
    return FIRST_INSTANCE;
}

/* This is an important function: It counts the number of white spaces in the string */

int GET_WHITE_SPACE_INSTANCES()
{
    int LENGTH_OF_STRING = strlen(STR_NEXT_LINE);
    int NUM_WHITE_SPACES = 0;
    for (int i = 0;i < LENGTH_OF_STRING;i++)
    {
        if (STR_NEXT_LINE[i] == ' ')
           NUM_WHITE_SPACES++;
    }
    
    return NUM_WHITE_SPACES;
}

/* Increments the value of Line_Number, which signify the creation of a new line */
void SET_NEW_LINE_NUMBER()
{
     // Line_Number = Line_Number + 1
     Line_Number++;
}

/* SPLIT_STRING Receives three parameters: STRING_ONE and STRING_TWO which are two strings which will receive
the split string, a character CHARACTER_TARGET where the string will be split apart, and STRING_MAIN, the string
to be divided */

void SPLIT_STRING(String STRING_ONE,String STRING_TWO, char CHARACTER_TARGET, String STRING_MAIN)
{
     int STRING_INDEX = 0;
     int STRING_LENGTH = strlen(STRING_MAIN);
     int STRING_ONE_INDEX = 0;
     int STRING_TWO_INDEX = 0;
     
     /* Initialize STRING_ONE and STRING_TWO as empty strings */
     strcpy(STRING_ONE,"");
     strcpy(STRING_TWO,"");
     
     while (STRING_INDEX < STRING_LENGTH && STRING_MAIN[STRING_INDEX] != CHARACTER_TARGET)
     {
           STRING_ONE[STRING_ONE_INDEX] = STRING_MAIN[STRING_INDEX];
           STRING_INDEX++;
           STRING_ONE_INDEX++;
     }
     STRING_ONE[STRING_ONE_INDEX] = '\0';
     STRING_INDEX++;
     
     while (STRING_INDEX < STRING_LENGTH-1) // (The -1 because we do not include the ; in the end)
     {
           STRING_TWO[STRING_TWO_INDEX] = STRING_MAIN[STRING_INDEX];
           STRING_INDEX++;
           STRING_TWO_INDEX++;
     }
     STRING_TWO[STRING_TWO_INDEX] = '\0';
}

/* SPLIT_COMMAS_FROM_STRING as ironic as the name sounds is actually just appends ' before
and after commas */

void SPLIT_COMMAS_FROM_STRING(String TARGET_STRING)
{
     String TEMP_STRING,NEW_STRING;
     
     int STRING_LENGTH = strlen(TARGET_STRING);
     int STRING_INDEX = 0;
     int NEW_STRING_INDEX = 0;
     
     strcpy(TEMP_STRING,TARGET_STRING);
     
     /* Initialize the string to be empty */
     strcpy(NEW_STRING,"");
     
     /* Initialize the new string to have a ' at the beginning */
     
     NEW_STRING[NEW_STRING_INDEX] = '\'';
     NEW_STRING_INDEX++;
     
     while (STRING_INDEX < STRING_LENGTH)
     {
           if (TEMP_STRING[STRING_INDEX] == ',')
           {
               NEW_STRING[NEW_STRING_INDEX] = '\'';
               NEW_STRING_INDEX++;
               NEW_STRING[NEW_STRING_INDEX] = ',';
               NEW_STRING_INDEX++;
               NEW_STRING[NEW_STRING_INDEX] = '\'';
               NEW_STRING_INDEX++;
           }
           else
           {
               NEW_STRING[NEW_STRING_INDEX] = TEMP_STRING[STRING_INDEX];
               NEW_STRING_INDEX++;
           }
           STRING_INDEX++;
     }
     NEW_STRING[NEW_STRING_INDEX] = '\0';
     strcpy(TARGET_STRING,NEW_STRING);
}

/* RETURN_ERROR_TYPE is a function which returns an error message into the
console*/

int CONTAINS_DOT(String TOKEN)
{
    int LENGTH = strlen(TOKEN);
    int INDEX = 0;
    int NUMBER_OF_DOTS = 0;
    
    for (INDEX = 0;INDEX < LENGTH;INDEX++)
    {
        if (TOKEN[INDEX] == '.')
           NUMBER_OF_DOTS++;
    }
    
    return NUMBER_OF_DOTS;
}

void RETURN_ERROR_TYPE(int ERROR_NUMBER,int COLUMN_NUMBER)
{
     int FIRST_INDEX_OF_SPACE = -1;
     
     switch (ERROR_NUMBER)
     {
            case TOO_MANY_TOKENS_ERROR:
                 fprintf(pFile,"Too many tokens detected");
                 break;
                 
            case TOO_LONG_INSTRUCTION_ERROR:
                 fprintf(pFile,"Instruction length exceeds 256 characters");
                 break;
                 
            case MISSING_SEMI_COLON:
                 fprintf(pFile,"Missing ';' at the end of the statement in column %d",COLUMN_NUMBER);
                 break;
            
            case EXCESS_STATEMENTS:
                 fprintf(pFile,"Excess statements starting at column %d after statement has been terminated",COLUMN_NUMBER+1);
                 break;
                 
            case UNIDENTIFIED_TOKEN:
                 STR_NEXT_LINE[COLUMN_NUMBER-1] = '\0';
                 fprintf(pFile,"Unknown command \'%s\', must be following an invalid syntax",STR_NEXT_LINE);
                 break;
     }
     
     return;
}

void RECOVER_FROM_ERROR (String STRING_TOKEN,int ERROR_TYPE)
{
     int STRING_LENGTH = strlen(STRING_TOKEN);
     int STRING_INDEX = 0;
                       String TEMP_STRING_1;
                  int TEMP_STRING_1_INDEX = 0;
                  String TEMP_STRING_2;
                  int TEMP_STRING_2_INDEX = 0;
                  int INDEX_OF_STRING;
     int LENGTH_OF_STRING = strlen(STRING_TOKEN);
     
     switch (ERROR_TYPE)
     {
            case 1:
                  
                  strcpy(TEMP_STRING_1,"");
                  strcpy(TEMP_STRING_2,"");
                 while ((IS_DIGIT(STRING_TOKEN[STRING_INDEX]) || IS_DOT_CHARACTER(STRING_TOKEN[STRING_INDEX]))&& STRING_INDEX < STRING_LENGTH)
                 {
                       TEMP_STRING_1[TEMP_STRING_1_INDEX] = STRING_TOKEN[STRING_INDEX];
                       STRING_INDEX++;
                       TEMP_STRING_1_INDEX++;
                 }
                 TEMP_STRING_1[TEMP_STRING_1_INDEX] = '\0';
                 
                 fprintf (pFile,"Appending ' ' before %c\n",STRING_TOKEN[STRING_INDEX]);
                 
                 while (STRING_INDEX < STRING_LENGTH)
                 {
                       TEMP_STRING_2[TEMP_STRING_2_INDEX] = STRING_TOKEN[STRING_INDEX];
                       STRING_INDEX++;
                       TEMP_STRING_2_INDEX++;
                 }
                 TEMP_STRING_2[TEMP_STRING_2_INDEX] = '\0';
                 
                 if (CONTAINS_DOT(TEMP_STRING_1) > 0)
                 {
                    PUSH("FLOATING");
                    fprintf (pFile,"Floating number token \"%s\" found\n",TEMP_STRING_1);
                 }
                 else
                 {
                    PUSH("INTEGER");
                    fprintf (pFile,"Integer number token \"%s\" found\n",TEMP_STRING_1);
                 }
                 fprintf (pFile,"Identified token \"%s\" found\n",TEMP_STRING_2);
                 PUSH("IDENTIFIER");
                 break;
            
            case 2:
                 fprintf (pFile,"Inserting \'\"\' at the end of the statement\n");
                 fprintf (pFile,"String Token \'%s\' detected\n",STRING_TOKEN);
                 PUSH("\"");
                 PUSH("CONST");
                 PUSH("\"");
                 break;
            
            case 3:
                 fprintf (pFile,"Replacing \'%c\' with \"\'\"\n",STRING_TOKEN[LENGTH_OF_STRING-1]);
                 STRING_TOKEN[LENGTH_OF_STRING-1] = '\'';
                 fprintf (pFile,"Character Token \'%c\' detected\n",STRING_TOKEN[1]);
                 PUSH("\'");
                 PUSH("CONST");
                 PUSH("\'");
                 break;
            
            case 4:
                 LENGTH_OF_STRING = strlen(STRING_TOKEN);
                 INDEX_OF_STRING = 0;
                 fprintf (pFile,"'.' Character cannot be used to start an identifier: Deleting '.'\n");
                 while (INDEX_OF_STRING < LENGTH_OF_STRING && IS_DOT_CHARACTER(STRING_TOKEN[INDEX_OF_STRING]))
                       INDEX_OF_STRING++;
                 if (INDEX_OF_STRING < LENGTH_OF_STRING)
                 {
                     if (IS_DIGIT(STRING_TOKEN[INDEX_OF_STRING]))
                     {
                        fprintf(pFile,"Number token \'");
                        PUSH("INTEGER");
                     }
                     else if (IS_ALPHA(STRING_TOKEN[INDEX_OF_STRING]))
                     {
                        fprintf(pFile,"Identifier token \'");
                        PUSH("IDENTIFIER");
                     }
                     while (INDEX_OF_STRING < LENGTH_OF_STRING)
                     {
                           fprintf (pFile,"%c",STRING_TOKEN[INDEX_OF_STRING]);
                           INDEX_OF_STRING++;
                     }
                     fprintf (pFile,"\' found\n");
                 }
                 break;
            
            case 5:
                 LENGTH_OF_STRING = strlen(STRING_TOKEN);
                 INDEX_OF_STRING = 0;
                 fprintf (pFile,"Multiple '.' Characters detected in a number token\n");
                 fprintf (pFile,"Replacing all '.' and alpha characters with 1's\n");
                 while (INDEX_OF_STRING < LENGTH_OF_STRING)
                 {
                       if (STRING_TOKEN[INDEX_OF_STRING] == '.' || IS_ALPHA(STRING_TOKEN[INDEX_OF_STRING]))
                          STRING_TOKEN[INDEX_OF_STRING] = '1';
                       INDEX_OF_STRING++;
                 }
                 fprintf (pFile,"Number token \'%s\' found\n",STRING_TOKEN);
                 PUSH("INTEGER");
                 break;
     }
}

int CHECK_VALIDITY (String IDENTIFIER_TOKEN)
{
     int LENGTH_OF_STRING = strlen(IDENTIFIER_TOKEN);
     int INDEX_OF_STRING = 1;
     
     bool IS_ALPHA_TOKEN = false;
     bool IS_INTEGER_TOKEN = false;
     bool IS_DECIMAL = false;
     
     if (LENGTH_OF_STRING > 0 && IDENTIFIER_TOKEN[0] == '.')
     {
        return -4;
     }
     if (IS_ALPHA(IDENTIFIER_TOKEN[0]))
        IS_ALPHA_TOKEN = true;
     else if (IS_DIGIT(IDENTIFIER_TOKEN[0]))
        IS_INTEGER_TOKEN = true;
     while (INDEX_OF_STRING < LENGTH_OF_STRING)
     {
           if (IS_DECIMAL == true && IS_DOT_CHARACTER(IDENTIFIER_TOKEN[INDEX_OF_STRING]))
              return -5;
           if (IS_INTEGER_TOKEN && IS_ALPHA(IDENTIFIER_TOKEN[INDEX_OF_STRING]))
              return -1;
           if (IS_INTEGER_TOKEN && IS_DOT_CHARACTER(IDENTIFIER_TOKEN[INDEX_OF_STRING]))
              IS_DECIMAL = true;   
           INDEX_OF_STRING++;
     }
     
     if (IS_ALPHA_TOKEN)
        return 1;
     else
        return 2;
}

void NOTIFY_OF_INTEGER_TOKEN(String INTEGER_TOKEN,int COLUMN_NUMBER)
{
     int NUMBER_OF_DOTS = CONTAINS_DOT(INTEGER_TOKEN);
     if (NUMBER_OF_DOTS == 0)
     {
        PUSH("INTEGER");
       // fprintf (pFile,"Integer token \"%s\" found on column number %d\n",INTEGER_TOKEN,COLUMN_NUMBER+1);
     }
     else
     {
        PUSH("FLOATING");
    //    fprintf (pFile,"Floating point token \"%s\" found on column number %d\n",INTEGER_TOKEN,COLUMN_NUMBER+1);
     }
}

void NOTIFY_OF_IDENTIFIER_TOKEN(String IDENTIFIER_TOKEN,int COLUMN_NUMBER)
{
     fprintf (pFile,"Identified token \"%s\" found on column number %d\n",IDENTIFIER_TOKEN,COLUMN_NUMBER+1);
}

/* NOTIFY_OF_TOKEN Notifies the user that the scanner has detected an operand token */
void NOTIFY_OF_TOKEN(int TYPE_OF_NOTIFY,char CHARACTER,int COLUMN_NUMBER)
{
     switch(TYPE_OF_NOTIFY)
     {
         case OPERATIONAL_TOKEN:
              fprintf(pFile,"Operational token \'%c\' detected at column number %d\n",CHARACTER,COLUMN_NUMBER+1);
              break;
         case RELATIONAL_OPERAND_TOKEN:
              fprintf(pFile,"Relational Operand token \'%c\' detected at column number %d\n",CHARACTER,COLUMN_NUMBER+1);
              break;
         case SEMICOLON_TOKEN:
              fprintf(pFile,"Semicolon token \'%c\' found at column number %d\n",CHARACTER,COLUMN_NUMBER+1);
              break;
         case COLON_TOKEN:
              fprintf(pFile,"Colon token \'%c\' found at column number %d\n",CHARACTER,COLUMN_NUMBER+1);
              break;
         case GROUP_TOKEN:
              fprintf(pFile,"Group token \'%c\' found at column number %d\n",CHARACTER,COLUMN_NUMBER+1);
              break;
     }
}

/* PARSE_STRING receives no parameters, but it makes use of the two
global variables Line_Number (indicates the line number) and STR_NEXT_LINE
which indicates the string to be scanned and analyzed */

void PARSE_STRING()
{
     int STRING_INDEX = 0;
     int LENGTH_OF_STRING = strlen(STR_NEXT_LINE);
     
     while (STRING_INDEX < LENGTH_OF_STRING)
     {
           char CHARACTER_TO_EVALUATE = STR_NEXT_LINE[STRING_INDEX];
    //       fprintf (pFile,"%c\n",CHARACTER_TO_EVALUATE);
           if (IS_ALPHA(CHARACTER_TO_EVALUATE) || IS_DIGIT(CHARACTER_TO_EVALUATE) || IS_DOT_CHARACTER(CHARACTER_TO_EVALUATE))
           {
              String TEMP_STRING;
              int TEMP_STRING_INDEX = 0;
              int START_COLUMN_NUMBER = STRING_INDEX;
              int ERROR_TYPE;
              strcpy(TEMP_STRING,"");
              do
              {
                   TEMP_STRING[TEMP_STRING_INDEX] = STR_NEXT_LINE[STRING_INDEX];
                   TEMP_STRING_INDEX++;
                   STRING_INDEX++;
                   CHARACTER_TO_EVALUATE = STR_NEXT_LINE[STRING_INDEX];
              } while (IS_ALPHA(CHARACTER_TO_EVALUATE) || IS_DIGIT(CHARACTER_TO_EVALUATE) || IS_DOT_CHARACTER(CHARACTER_TO_EVALUATE));
              TEMP_STRING[TEMP_STRING_INDEX] = '\0';
              
              ERROR_TYPE = CHECK_VALIDITY(TEMP_STRING);
            //  fprintf (pFile,"State type: %d\n",ERROR_TYPE);
              if (ERROR_TYPE > 0)
              {
                 if (ERROR_TYPE == 1)
                 {
               //     NOTIFY_OF_IDENTIFIER_TOKEN(TEMP_STRING,START_COLUMN_NUMBER);
                    PUSH(TEMP_STRING);
                 }
                 else if (ERROR_TYPE == 2)
                    NOTIFY_OF_INTEGER_TOKEN(TEMP_STRING,START_COLUMN_NUMBER);
              }
              else
              {
                  fprintf (pFile,"ERROR DETECTED\n");
                  if (ERROR_TYPE == -1)
                     RECOVER_FROM_ERROR(TEMP_STRING,ERROR_TYPE*-1);
                  else if (ERROR_TYPE == -4)
                  {
                     if (CONTAINS_DOT(TEMP_STRING) > 1)
                        RECOVER_FROM_ERROR(TEMP_STRING,5);
                     else
                        RECOVER_FROM_ERROR(TEMP_STRING,ERROR_TYPE*-1);
                  }
                  else if (ERROR_TYPE == -5)
                     RECOVER_FROM_ERROR(TEMP_STRING,ERROR_TYPE*-1);
              }
           }
           else if (IS_DOUBLE_QUOTE(CHARACTER_TO_EVALUATE) || IS_SINGLE_QUOTE(CHARACTER_TO_EVALUATE))
           {
                int START_COLUMN_NUMBER = STRING_INDEX+1;
                if (IS_DOUBLE_QUOTE(CHARACTER_TO_EVALUATE))
                { 
                     STRING_INDEX++;
                     String TEMP_STRING;
                     int TEMP_STRING_INDEX = 0;
                     CHARACTER_TO_EVALUATE = STR_NEXT_LINE[STRING_INDEX];
                     while (STRING_INDEX < LENGTH_OF_STRING && !IS_DOUBLE_QUOTE(CHARACTER_TO_EVALUATE))
                     {
                           TEMP_STRING[TEMP_STRING_INDEX] = CHARACTER_TO_EVALUATE;
                           STRING_INDEX++;
                           TEMP_STRING_INDEX++;
                           CHARACTER_TO_EVALUATE = STR_NEXT_LINE[STRING_INDEX];
                     }
                     TEMP_STRING[TEMP_STRING_INDEX] = '\0';
                     if (CHARACTER_TO_EVALUATE != '"')
                     {
                        fprintf (pFile,"ERROR DETECTED at column %d! Unterminated String syntax\n",START_COLUMN_NUMBER);
                        RECOVER_FROM_ERROR(TEMP_STRING,2);
                     }
                     else
                     {
                     //    fprintf (pFile,"String Token \'%s\' detected at column %d\n",TEMP_STRING,START_COLUMN_NUMBER);
                         STRING_INDEX++;
                         PUSH("\"");
                         PUSH("CONST");
                         PUSH("\"");
                     }
                }
                else if (IS_SINGLE_QUOTE(CHARACTER_TO_EVALUATE))
                {
                     STRING_INDEX++;
                     String TEMP_STRING;
                     int TEMP_STRING_INDEX = 0;
                     CHARACTER_TO_EVALUATE = STR_NEXT_LINE[STRING_INDEX];
                     while (TEMP_STRING_INDEX < 2 && STRING_INDEX < LENGTH_OF_STRING)
                     {
                           TEMP_STRING[TEMP_STRING_INDEX] = CHARACTER_TO_EVALUATE;
                           STRING_INDEX++;
                           TEMP_STRING_INDEX++;
                           CHARACTER_TO_EVALUATE = STR_NEXT_LINE[STRING_INDEX];
                     }
                     TEMP_STRING[TEMP_STRING_INDEX] = '\0';
                     if (!IS_SINGLE_QUOTE(TEMP_STRING[TEMP_STRING_INDEX-1]))
                        RECOVER_FROM_ERROR(TEMP_STRING,3);
                     else
                     {
                    //    TEMP_STRING[TEMP_STRING_INDEX] = '\0';
                        PUSH("\'");
                        PUSH(TEMP_STRING);
                        PUSH("\'");
                        fprintf (pFile,"Character token \'%s detected at column %d\n",TEMP_STRING,START_COLUMN_NUMBER);
                     //   STRING_INDEX++;
                     }
                }
           }
           else
           {
               String temp;
               temp[0] = CHARACTER_TO_EVALUATE;
               temp[1] = '\0';
               switch (CHARACTER_TO_EVALUATE)
               {
                      case '/':
                      case '+':
                      case '-':
                      case '*':
                      case '%':
                           PUSH(temp);
                           break;
                      
                      case '&':
                           PUSH(temp);
                           break;
                           
                      case '>':
                      case '=':
                      case '<':
                      if (STRING_INDEX+1 < LENGTH_OF_STRING && STR_NEXT_LINE[STRING_INDEX+1] == '=')
                      {
                                         STRING_INDEX++;
                                         temp[1] = '=';
                                         temp[2] = '\0';
                                         PUSH(temp);
                      }     
                      else
                          PUSH(temp);
                           break;
                      
                      case ';':
                          // NOTIFY_OF_TOKEN(SEMICOLON_TOKEN,CHARACTER_TO_EVALUATE,STRING_INDEX);
                          PUSH(temp);
                           PRINT_STACK_CONTENTS();
                           break;
                      
                      case ')':
                           PUSH(temp);
                           PRINT_STACK_CONTENTS();
                           break;
                           
                      case ':':
                           PUSH(temp);
                           break;
                      
                      case '(':
                      case '[':
                      case ']':
                           PUSH(temp);
                           break;
                      
                      case '{':
                           PUSH(temp);
                           PUSH_BRACKET();
                           break;
                      
                      case '}':
                           PUSH(temp);
                           if (!POP_BRACKET())
                              fprintf (pFile,"ERROR: EXCESS '}'");
                           break;
                           
                      case ',':
                      case '#':
                           PUSH(temp);
                           break;
                      
               }
               STRING_INDEX++;
           }
     }
}

void TRIM_STRING()
{
     String TEMPORARY_STRING,NEW_STRING;
     bool FIRST_SPACES_FLAG = true; // mark the flag as true
     int String_Index = 0; // The index of the string's iteration
     int New_String_Index = 0; // The target index of new characters given to NEW_STRING
     int Length_of_string = strlen(STR_NEXT_LINE);
     
     /* Copy the contents of the real string into a temporary string */
     strcpy(TEMPORARY_STRING,STR_NEXT_LINE);
     
     /* Initialize NEW_STRING to be an empty String */
     strcpy(NEW_STRING,"");
     
     /* Check the beginning if we have spaces in the beginning, and ignore them */
     while (String_Index < Length_of_string)
     {
           if (FIRST_SPACES_FLAG && (TEMPORARY_STRING[String_Index] == ' ' || TEMPORARY_STRING[String_Index] == '\t'))
           {
                 String_Index++;
                 continue;
           }
           FIRST_SPACES_FLAG = false;
           if ((String_Index - 1) >= 0 && TEMPORARY_STRING[String_Index] == ' ' && TEMPORARY_STRING[String_Index-1] == ' ')
           {
                 String_Index++;
                 continue;
           }
           else
           {
                 NEW_STRING[New_String_Index] = TEMPORARY_STRING[String_Index];
               //  if (NEW_STRING[New_String_Index] == '\'')
                //    NEW_STRING[New_String_Index] = '\"';
                 New_String_Index++;
                 String_Index++;
           }
     }
     
     if (New_String_Index == 0); // Empty string? Do nothing
     /* Is there a single trailing space at the end of the string? */
     else if (NEW_STRING[New_String_Index-1] == ' ' || NEW_STRING[New_String_Index-1] == '\t')
     {
          NEW_STRING[New_String_Index-1] = '\0';
          New_String_Index--;
     }
     /* Otherwise, just add the terminating character */
     else
         NEW_STRING[New_String_Index] = '\0';
     strcpy(STR_NEXT_LINE,NEW_STRING);
}

int main(int argc,char *argv[]) {
    
    int i=0;
    
    if (argc <= 1)
    {
             printf ("Please run compile_parser.exe first, and parse the code before running the interpreter. Thanks!\n");
             printf ("- Kelvin Chua and Patrick Gotauco...\n");
             printf ("The dudes who wrote this printf statement, and was their only contribution\n");
             getch();
    }
    else
    {
            INIT_MAIN_PROGRAM_CONTENTS();
            if (strcmp(argv[1],"TRUE") == 0)
            {
                printf ("PARSING SUCCESSFUL! INTERPRETER STARTED\n");
                printf ("\n");
                FILE *file,*pFile;
                file = fopen(INPUT_FILENAME,"rt");
                
                pFile = fopen(OUTPUT_FILENAME,"wt");
                
               while (fgets(STR_NEXT_LINE, 1000, file) != NULL)
               {
                     TRIM_STRING();
                     fprintf (pFile,STR_NEXT_LINE);
               }
               fclose(file);
               fclose(pFile);
           //    getch(); 
               system("cls");
            }
            else
            {
                printf ("PARSING UNSUCCESSFUL! INTERPRETER NOT STARTED\n"); 
            }
    }
    //getch();
}
