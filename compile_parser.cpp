#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <conio.h>

#define INPUT_FILENAME "code_file.gcc"
// #define INPUT_FILENAME "input.txt"
#define OUTPUT_FILENAME "output.txt"
#define ERROR_LOG       "ERROR.log"

#define SAFE_RELEASE
#define TAKE_INPUT_FROM_TEXT_FILE
#define WRITE_OUTPUT_TO_TEXT_FILE

//#define INTERPRETER
#define TESTING_FUNCTIONS

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

typedef enum
{
        NUMBER_FORMAT_EXCEPTION = 0,
        VARIABLE_MISMATCH_EXCEPTION,
        ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION,
        DIVISION_BY_ZERO_EXCEPTION
};

FILE *pFile;
FILE *pErrorFile;
FILE *pTraceFile;

typedef char String[1001]; 
/* Strings are 1000 length at maximum, but the convention is that it will be scanned until the 256th character*/

/* Global variables */
   String STR_NEXT_LINE;
   long Line_Number;
/* End global variables */

String Queue[100];
String Symbol[100];
String Stack[100000];

String Stack2[100000];
String Variable_List[100000];
int Variable_Type[100000];

String Bracket_Stack;
String Function_List[1000];
String Constant_List[100000];
String Array_List[1000];

int array_index;
int const_index;
int func_index;

int QUEUE_INDEX;
int STACK_INDEX;
int STACK_2_INDEX;
int SYMBOL_INDEX;
int BRACKET_INDEX;
int VARIABLE_LIST_INDEX;
int NUM_IFS;
int num_errors;

bool RUNTIME_ERROR;
bool HAS_ERROR;
bool WATCH_AND_TRACE;
bool IMPROVED_WATCH_AND_TRACE;

int EXCEPTION_TYPE;

String EVALUATING_FUNCTION_NAME;

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

VARIABLE VARIABLES[10000];

int VARIABLES_INDEX;

typedef struct
{
        String const_name;
        int const_type;
        String const_value;
} CONSTANT;

CONSTANT CONSTANTS[10000];

int CONSTANTS_INDEX;

typedef struct
{
        String array_name;
        int array_type;
        String array_values[1000];
        int array_size;
} ARRAY;

ARRAY ARRAYS[100];

int ARRAYS_INDEX;

void NEW_ARRAY(int type,String name,int size)
{
     strcpy(ARRAYS[ARRAYS_INDEX].array_name,name);
     ARRAYS[ARRAYS_INDEX].array_size = size;
     ARRAYS[ARRAYS_INDEX].array_type = type;
     ARRAYS_INDEX++;
}

void UPDATE_ARRAY(String name,int index,String value)
{
     int i;
   //  printf ("Updating %s index %d with value %s\n",name,index,value);
     for (i = 0;i < ARRAYS_INDEX;i++)
     {
         if (strcmp(ARRAYS[i].array_name,name) == 0)
         {
             if (index < 0 || index > ARRAYS[i].array_size)
             {
                       RUNTIME_ERROR = true;
                       EXCEPTION_TYPE = ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION;
                       if (WATCH_AND_TRACE)
                       {
                                           printf ("<RUNTIME_ERROR: ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION>\n");
                                           getch();
                       }
                       return;
             }
             strcpy(ARRAYS[i].array_values[index],value);
         }
     }
}

void GET_ARRAY(String name,int index,String output)
{
     int i;
     for (i = 0;i < ARRAYS_INDEX;i++)
     {
         if (strcmp(ARRAYS[i].array_name,name) == 0)
         {
             if (index < 0 || index > ARRAYS[i].array_size)
             {
                       RUNTIME_ERROR = true;
                       EXCEPTION_TYPE = ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION;
                       if (WATCH_AND_TRACE)
                       {
                              printf ("<RUNTIME_ERROR: ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION>\n");
                              getch();
                       }
                       return;
             }
             strcpy(output,ARRAYS[i].array_values[index]);
         }
     }
}

void ADD_ARRAY_LIST(String cand)
{
     strcpy(Array_List[array_index],cand);
     array_index++;
   //  printf ("Array with name %s added\n",cand);
   //  getch();
}

bool contains_array_list(String cand)
{
     int i;
     for (i = 0;i < array_index;i++)
     {
         if (strcmp(cand,Array_List[i]) == 0)
            return true;
     }
     return false;
}

/************* Functions ****************/
typedef struct
{
		String name;
        String statements[1000];
        int num_statements;
        String parameters[1000];
        int parameter_types[1000];
        int num_parameters;
        int statement_indx;
        int return_type;
        String return_value;
} FUNCTION;

FUNCTION FUNCTIONS[100];
int FUNCTIONS_INDEX;

int ADD_FUNCTION(String name)
{
	for (int i = 0;i < FUNCTIONS_INDEX;i++)
	{
		if (strcmp(FUNCTIONS[i].name,name) == 0)
			return i;
	}
	strcpy(FUNCTIONS[FUNCTIONS_INDEX].name,name);
//	printf ("New function added: %s\n",FUNCTIONS[FUNCTIONS_INDEX].name);
	FUNCTIONS[FUNCTIONS_INDEX].num_statements = 0;
	FUNCTIONS[FUNCTIONS_INDEX].num_parameters = 0;
	FUNCTIONS[FUNCTIONS_INDEX].statement_indx = 0;
	FUNCTIONS_INDEX++;
	return FUNCTIONS_INDEX-1;
}

void UPDATE_FUNCTION_TYPE(String name,int val)
{
     int i = ADD_FUNCTION(name);
     FUNCTIONS[i].return_type = val;
}

int GET_FUNCTION_TYPE(String name)
{
    int i = ADD_FUNCTION(name);
    return FUNCTIONS[i].return_type;
}

int FUNCTION_AVAILABLE(String name)
{
    int i;
   	for (i = 0;i < FUNCTIONS_INDEX;i++)
	{
		if (strcmp(FUNCTIONS[i].name,name) == 0)
			return i;
	}
	return -1;
}

/*************Parsing stage ********************/

bool contains_function_name(String cand)
{
	int i;
	for (i = 0;i < func_index;i++)
	{
		if (strcmp(Function_List[i],cand) == 0)
			return true;
	}
	return false;
}

void CLEAR_FUNCTION_LIST()
{
     func_index = 0;
}

void ADD_FUNCTION_LIST(String cand)
{
     if (contains_function_name(cand))
     {
                        
                        fprintf(pErrorFile,"Line Number %d: %s\n",Line_Number,STR_NEXT_LINE);
                        fprintf(pErrorFile,"Message: Redeclaraion of %s\n",cand);
                        fprintf(pErrorFile,"\n");
                        HAS_ERROR = true;
                        num_errors++;
                    //    printf ("eRRORED?\n");
                    //    getch();
                        return;
     }
     else
     {
	     strcpy(Function_List[func_index],cand);
	     func_index++;
     }
}

void ADD_CONSTANT_LIST(String cand)
{
	strcpy(Constant_List[const_index],cand);
	const_index++;
}

bool contains_constant_name(String cand)
{
	int i;
	for (i = 0;i < const_index;i++)
	{
		if (strcmp(Constant_List[i],cand) == 0)
			return true;
	}
	return false;
}

bool IS_CONSTANT(String cand)
{
     if (contains_constant_name(cand))
        return true;
     return false;
}

/************* End functions ******************/

String operator_stack;
int operator_stack_index;
String operator_stack_float;
int operator_stack_float_index;

int number_stack[100];
int number_stack_index;

float number_stack_float[100];
int number_stack_float_index;

int kalbo_index;

void EVALUATE_EQUATION(String Source,String output);
void INFIX_TO_POSTFIX(String Source,String final_result);
void OPERATOR_REFRESH();
char OPERATOR_POP();
char OPERATOR_PEEK();
void OPERATOR_PUSH(char ch);
int NUMBER_POP();
void PRINT_NUMBER_STACK();
void NUMBER_PUSH(int num);

void EVALUATE_EQUATION_FLOAT(String Source,String output);
void INFIX_TO_POSTFIX_FLOAT(String Source,String final_result);
void OPERATOR_REFRESH_FLOAT();
char OPERATOR_POP_FLOAT();
char OPERATOR_PEEK_FLOAT();
void OPERATOR_PUSH_FLOAT(char ch);
float NUMBER_POP_FLOAT();
void PRINT_NUMBER_STACK_FLOAT();
void NUMBER_PUSH_FLOAT(int num);

bool Contains (String Cand);
bool ALREADY_DECLARED (String Cand);
bool isHigher(char c1,char c2);

bool IS_EQUATION(String str)
{
     int i, length = strlen(str);
     if (str[0] >= 'a' && str[0] <= 'z' || str[0] >= 'A' && str[0] <= 'Z')
        return true;
     for (i = 0;i < length;i++)
     {
         if (str[i] == '+' || str[i] == '-' || str[i] == '*' || str[i] == '/' || str[i] == '$')
            return true;
     }
     return false;
}

bool ALREADY_DECLARED(String Cand)
{
}

void REMOVE_LOCAL_VARIABLES(int terminate_index)
{
     VARIABLES_INDEX = terminate_index;
}

void WRITE_VARIABLE_DATA()
{
     int i;
     pTraceFile = fopen("trace.log","wt");
     fprintf (pTraceFile,"Variables: ");
     fprintf (pTraceFile,"\n");
     for (i = VARIABLES_INDEX - 1;i >= 0;i--)
     {
         fprintf(pTraceFile,"%s = %s\n",VARIABLES[i].var_name,VARIABLES[i].var_value);
     }
     fclose(pTraceFile);
}

void NEW_VARIABLE(int type,String name,String value)
{
     VARIABLE temp;
     temp.var_type = type;
     strcpy(temp.var_name,name);
     strcpy(temp.var_value,value);
     if ((temp.var_type == INT || temp.var_type == FLOAT) && IS_EQUATION(temp.var_value))
     {
        String str_new_value;
        strcpy(str_new_value,temp.var_value);
        if (temp.var_type == INT)
           EVALUATE_EQUATION(temp.var_value,str_new_value);
        else if (temp.var_type == FLOAT)
           EVALUATE_EQUATION_FLOAT(temp.var_value,str_new_value);
        strcpy(temp.var_value,str_new_value);
     }
     VARIABLES[VARIABLES_INDEX] = temp;
     if (WATCH_AND_TRACE)
     {
        printf ("<Loaded new variable: %s with value %s>\n",name,value);
        getch();
     }
     VARIABLES_INDEX++;
     if (IMPROVED_WATCH_AND_TRACE)
     {
        printf ("<BREAKPOINT: Declared a new variable %s with value %s>\n",name,value);
        WRITE_VARIABLE_DATA();
        getch();
     }
}

int GET_VARIABLE_TYPE(String name)
{
     int i;
     for (i = 0;i < VARIABLES_INDEX;i++)
     {
         if (strcmp(name,VARIABLES[i].var_name) == 0)
            return VARIABLES[i].var_type;
     }
     return -1;
}

void NEW_CONSTANT(String name,String value)
{
     CONSTANT temp;
//     temp.const_type = type;
     strcpy(temp.const_name,name);
     strcpy(temp.const_value,value);/*
     if ((temp.const_type == 0 || temp.const_type == 2) && IS_EQUATION(temp.const_value))
     {
        String str_new_value;
        strcpy(str_new_value,temp.const_value);
        EVALUATE_EQUATION(temp.const_value,str_new_value);
        strcpy(temp.const_value,str_new_value);
     }*/
     CONSTANTS[CONSTANTS_INDEX] = temp;
     CONSTANTS_INDEX++;
}

void GET_CONSTANT_VALUE(String name,String value)
{
     int i = 0;
     for (i = CONSTANTS_INDEX-1;i >= 0;i--)
     {
         if (strcmp(CONSTANTS[i].const_name,name) == 0)
         {
            strcpy(value,CONSTANTS[i].const_value);
            if (CONSTANTS[i].const_type == INT && IS_EQUATION(CONSTANTS[i].const_value))
            {
               String str_new_value;
               strcpy(str_new_value,CONSTANTS[i].const_value);
               EVALUATE_EQUATION(CONSTANTS[i].const_value,str_new_value);
               strcpy(CONSTANTS[i].const_value,str_new_value);
               strcpy(value,CONSTANTS[i].const_value);
            }
            return;
         }
     }
}

bool CONTAINS_SAME_VARIABLE_NAME(String name,String value)
{
     return true;
}

bool VALID_INTEGER(String cand)
{
     int i = 0,v = strlen(cand);
    // printf ("Checking validity\n");
     for (i = 0;i < v;i++)
     {
         if ((cand[i] >= '0' && cand[i] <= '9') || cand[i] == '-');
         else return false;
     }
     return true;
}

void UPDATE_VARIABLE(String name,String value)
{
     int i = 0;
     for (i = VARIABLES_INDEX-1;i >= 0;i--)
     {
         if (strcmp(VARIABLES[i].var_name,name) == 0)
         {
            strcpy(VARIABLES[i].var_value,value);
            if ((VARIABLES[i].var_type == INT || VARIABLES[i].var_type == FLOAT) && IS_EQUATION(VARIABLES[i].var_value))
            {
               String str_new_value;
               strcpy(str_new_value,VARIABLES[i].var_value);
               if (VARIABLES[i].var_type == INT)
                  EVALUATE_EQUATION(VARIABLES[i].var_value,str_new_value);
               else
                  EVALUATE_EQUATION_FLOAT(VARIABLES[i].var_value,str_new_value);
               if (RUNTIME_ERROR)
                  return;
               strcpy(VARIABLES[i].var_value,str_new_value);
               strcpy(value,VARIABLES[i].var_value);
              
             //  printf ("%s\n",value);
            }
            if (WATCH_AND_TRACE)
            {
                        printf ("<Update variable: %s with value %s>\n",name,value);
                        getch();
            }
            if (IMPROVED_WATCH_AND_TRACE)
            {
                        printf ("<BREAKPOINT: Updated variable %s with value %s>\n",name,value);
                        WRITE_VARIABLE_DATA();
                        getch();
            }
            return;
         }
     }
}

void GET_VARIABLE_VALUE(String name,String value)
{
     int i = 0;
     for (i = VARIABLES_INDEX-1;i >= 0;i--)
     {
         if (strcmp(VARIABLES[i].var_name,name) == 0)
         {
            strcpy(value,VARIABLES[i].var_value);
            if ((VARIABLES[i].var_type == INT ||VARIABLES[i].var_type == FLOAT) && IS_EQUATION(VARIABLES[i].var_value))
            {
               String str_new_value;
               strcpy(str_new_value,VARIABLES[i].var_value);
               if (VARIABLES[i].var_type == INT)
                  EVALUATE_EQUATION(VARIABLES[i].var_value,str_new_value);
               else if (VARIABLES[i].var_type == FLOAT)
                  EVALUATE_EQUATION_FLOAT(VARIABLES[i].var_value,str_new_value);
               strcpy(VARIABLES[i].var_value,str_new_value);
               strcpy(value,VARIABLES[i].var_value);
            }
            if (WATCH_AND_TRACE)
            {
                        printf ("<Obtained variable: %s with value %s>\n",name,value);
                        getch();
            }
            return;
         }
     }
     
     for (i = CONSTANTS_INDEX-1;i >= 0;i--)
     {
         if (strcmp(CONSTANTS[i].const_name,name) == 0)
         {
       //     printf ("CONSTANT OBTAINED!\n");
            if (WATCH_AND_TRACE)
            {
                        printf ("<Obtained variable: %s with value %s>\n",name,value);
                        getch();
            }
            strcpy(value,CONSTANTS[i].const_value);
            return;
         }
     }
}

/* For parsing stage */

int eval (String Cand)
{
     if (strcmp(Cand,"int") == 0)
        return INT;
     else if (strcmp(Cand,"char") == 0)
        return CHAR;
     else if (strcmp(Cand,"float") == 0)
        return FLOAT;
     else if (strcmp(Cand,"string") == 0)
        return STRING;
     else if (strcmp(Cand,"boolean") == 0)
        return BOOLEAN;
     else
        return -1;
}

int getVariableType(String Cand)
{
    int i;
    for (i = VARIABLE_LIST_INDEX-1;i >= 0;i--)
    {
        if (strcmp(Cand,Variable_List[i]) == 0)
           return Variable_Type[i];
    }
}

void ADD_VARIABLE(String Cand,String var_type)
{
     if (eval(Cand) != -1)
        return;
     if (Contains(Cand))
     {
                        fprintf(pErrorFile,"Line Number %d: %s\n",Line_Number,STR_NEXT_LINE);
                        fprintf(pErrorFile,"Message: Redeclaraion of %s\n",Cand);
                        fprintf(pErrorFile,"\n");
                        HAS_ERROR = true;
                        num_errors++;
                        return;
     }
     else
     {
         strcpy(Variable_List[VARIABLE_LIST_INDEX],Cand);
         Variable_Type[VARIABLE_LIST_INDEX] = eval(var_type);
       //  printf ("Declaration of new variable %s with %d\n",Variable_List[VARIABLE_LIST_INDEX],Variable_Type[VARIABLE_LIST_INDEX]);
         VARIABLE_LIST_INDEX++;
        // getch();
     }
}

void CLEAR_LOCAL_VARIABLES()
{
     VARIABLE_LIST_INDEX = 0;
}

void PRINTS()
{
     int i;
     for (i = 0;i < VARIABLE_LIST_INDEX;i++)
         printf("%s\n",Variable_List[i]);
}
bool Contains(String Cand)
{
     int i = 0;
     for (i = 0;i < VARIABLE_LIST_INDEX;i++)
     {
         if (strcmp(Variable_List[i],Cand) == 0)
            return true;
     }

     if (contains_constant_name(Cand))
        return true;
        
     return false;
}


/************* Variable storage ***************/

void INSERT_TO_STACK_2(String CAND_STRING)
{
     strcpy(Stack2[STACK_2_INDEX],CAND_STRING);
     STACK_2_INDEX++;
}

void PRINT_STACK_CONTENTS()
{
     for (int i = 0;i < STACK_INDEX;i++)
         fprintf(pFile,"%s ",Stack[i]);
     fprintf (pFile,"\n");
}

bool IS_RESERVED(String CAND)
{
     int i;
     for (i = 0;i < QUEUE_INDEX;i++)
         if (strcmp(Queue[i],CAND) == 0)
            return true;
     return false;
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
     
     if (contains_function_name(CAND_STRING))
        strcpy(Stack[STACK_INDEX],"FUNCTION_CALL");
     else if (Contains(CAND_STRING))
        strcpy(Stack[STACK_INDEX],"IDENTIFIER");
     else if (contains_array_list(CAND_STRING))
         strcpy(Stack[STACK_INDEX],"array_var");
   //  if (contains_function_name(CAND_STRING))
     //   strcpy(Stack[STACK_INDEX],"FUNCTION_CALL");
     else
        strcpy(Stack[STACK_INDEX],"UNDEFINED");
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

void GET_CONTENTS(String s1,String s2,int cand)
{
     strcpy(s1,Stack2[cand]);
     strcpy(s2,Stack2[cand+1]);
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
     VARIABLE_LIST_INDEX = 0;
     VARIABLES_INDEX = 0;
     STACK_2_INDEX = 0;
     NUM_IFS = 0;
     HAS_ERROR = false;
     RUNTIME_ERROR = false;
     WATCH_AND_TRACE = false;
     IMPROVED_WATCH_AND_TRACE = false;
     num_errors = 0;
     operator_stack_index = 0;
     number_stack_index = 0;
     operator_stack_float_index = 0;
     number_stack_float_index = 0;
     func_index = 0;
     array_index = 0;
     ARRAYS_INDEX = 0;
     FUNCTIONS_INDEX = 0;
	 kalbo_index = 0;
	 
	 strcpy (EVALUATING_FUNCTION_NAME,"");
     ENQUEUE("main","main_optr");
     ENQUEUE("int","int_optr");
     ENQUEUE("char","char_optr");
     ENQUEUE("float","float_optr");
     ENQUEUE("string","string_optr");
     ENQUEUE("boolean","boolean_optr");
     ENQUEUE("void","void_optr");
     ENQUEUE("if","if_optr");
     ENQUEUE("else","else_optr");
     ENQUEUE("else_if","else_if_optr");
     ENQUEUE("do_while","do_while_optr");
     ENQUEUE("return","return_optr");
     ENQUEUE("for","for_optr");
     ENQUEUE("test","test_optr");
     ENQUEUE("then","then_optr");
     ENQUEUE("set","set_optr");
     ENQUEUE("while","while_optr");
     ENQUEUE("read","read_optr");
 //    ENQUEUE("pneumonoultramicroscopicsilicovolcanoconiosis","write_optr");
     ENQUEUE("write","write_optr");
     ENQUEUE("(","op_paren");
     ENQUEUE(")","cl_paren");
     ENQUEUE("[","op_brack");
     ENQUEUE("]","cl_brack");
     ENQUEUE("{","op_brace");
     ENQUEUE("}","cl_brace");
     ENQUEUE(";","sem_col");
     ENQUEUE("+","optr");
     ENQUEUE("-","optr");
     ENQUEUE("*","optr");
     ENQUEUE("/","optr");
     ENQUEUE("$","optr");
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
     ENQUEUE("CONST","CONST");
     ENQUEUE("function","func");
     ENQUEUE("as","as");
     ENQUEUE("declare","declare");
     ENQUEUE("endif","endif");
     ENQUEUE("//","comm_op");
     ENQUEUE("array","array_optr");
     ENQUEUE("+=","inc_optr");
     ENQUEUE("-=","inc_optr");
     ENQUEUE("*=","inc_optr");
     ENQUEUE("/=","inc_optr");
     ENQUEUE("%=","inc_optr");
     ENQUEUE("using","using_optr");
     ENQUEUE("assign","assign_optr");
     ENQUEUE("setArrayValue","set_array");
     ENQUEUE("getArrayValue","get_array");
     ENQUEUE("catch","catch_optr");
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

bool IS_UNDERSCORE (char CHARACTER)
{
     if (CHARACTER == '_')
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
              //   fprintf(pFile,"Too many tokens detected");
                 break;
                 
            case TOO_LONG_INSTRUCTION_ERROR:
              //   fprintf(pFile,"Instruction length exceeds 256 characters");
                 break;
                 
            case MISSING_SEMI_COLON:
            //     fprintf(pFile,"Missing ';' at the end of the statement in column %d",COLUMN_NUMBER);
                 break;
            
            case EXCESS_STATEMENTS:
             //    fprintf(pFile,"Excess statements starting at column %d after statement has been terminated",COLUMN_NUMBER+1);
                 break;
                 
            case UNIDENTIFIED_TOKEN:
                 STR_NEXT_LINE[COLUMN_NUMBER-1] = '\0';
           //      fprintf(pFile,"Unknown command \'%s\', must be following an invalid syntax",STR_NEXT_LINE);
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
                 
             //    fprintf (pFile,"Appending ' ' before %c\n",STRING_TOKEN[STRING_INDEX]);
                 
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
              //      fprintf (pFile,"Floating number token \"%s\" found\n",TEMP_STRING_1);
                 }
                 else
                 {
                    PUSH("INTEGER");
              //      fprintf (pFile,"Integer number token \"%s\" found\n",TEMP_STRING_1);
                 }
              //   fprintf (pFile,"Identified token \"%s\" found\n",TEMP_STRING_2);
                 PUSH("IDENTIFIER");
                 break;
            
            case 2:
              //   fprintf (pFile,"Inserting \'\"\' at the end of the statement\n");
              //   fprintf (pFile,"String Token \'%s\' detected\n",STRING_TOKEN);
                 PUSH("\"");
                 PUSH("CONST");
                 PUSH("\"");
                 break;
            
            case 3:
              //   fprintf (pFile,"Replacing \'%c\' with \"\'\"\n",STRING_TOKEN[LENGTH_OF_STRING-1]);
                 STRING_TOKEN[LENGTH_OF_STRING-1] = '\'';
              //   fprintf (pFile,"Character Token \'%c\' detected\n",STRING_TOKEN[1]);
                 PUSH("\'");
                 PUSH("CONST");
                 PUSH("\'");
                 break;
            
            case 4:
                 LENGTH_OF_STRING = strlen(STRING_TOKEN);
                 INDEX_OF_STRING = 0;
              //   fprintf (pFile,"'.' Character cannot be used to start an identifier: Deleting '.'\n");
                 while (INDEX_OF_STRING < LENGTH_OF_STRING && IS_DOT_CHARACTER(STRING_TOKEN[INDEX_OF_STRING]))
                       INDEX_OF_STRING++;
                 if (INDEX_OF_STRING < LENGTH_OF_STRING)
                 {
                     if (IS_DIGIT(STRING_TOKEN[INDEX_OF_STRING]))
                     {
                  //      fprintf(pFile,"Number token \'");
                        PUSH("INTEGER");
                     }
                     else if (IS_ALPHA(STRING_TOKEN[INDEX_OF_STRING]))
                     {
                  //      fprintf(pFile,"Identifier token \'");
                        PUSH("IDENTIFIER");
                     }
                     while (INDEX_OF_STRING < LENGTH_OF_STRING)
                     {
                           fprintf (pFile,"%c",STRING_TOKEN[INDEX_OF_STRING]);
                           INDEX_OF_STRING++;
                     }
                //     fprintf (pFile,"\' found\n");
                 }
                 break;
            
            case 5:
                 LENGTH_OF_STRING = strlen(STRING_TOKEN);
                 INDEX_OF_STRING = 0;
             //    fprintf (pFile,"Multiple '.' Characters detected in a number token\n");
            //     fprintf (pFile,"Replacing all '.' and alpha characters with 1's\n");
                 while (INDEX_OF_STRING < LENGTH_OF_STRING)
                 {
                       if (STRING_TOKEN[INDEX_OF_STRING] == '.' || IS_ALPHA(STRING_TOKEN[INDEX_OF_STRING]))
                          STRING_TOKEN[INDEX_OF_STRING] = '1';
                       INDEX_OF_STRING++;
                 }
          //       fprintf (pFile,"Number token \'%s\' found\n",STRING_TOKEN);
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

bool isDeclaring (String Cand)
{
     if (strcmp(Cand,"int") == 0)
        return true;
     else if (strcmp(Cand,"char") == 0)
        return true;
     else if (strcmp(Cand,"float") == 0)
        return true;
     else if (strcmp(Cand,"string") == 0)
        return true;
     else if (strcmp(Cand,"boolean") == 0)
        return true;
     else
        return false;
}

bool isDeclaringArray (String Cand)
{
     if (strcmp(Cand,"array") == 0)
        return true;
     else
         return false;
}

bool isDeclaringFunction (String Cand)
{
     if (strcmp(Cand,"function") == 0 || strcmp(Cand,"main") == 0)
        return true;
     else
         return false;
}
/* PARSE_STRING receives no parameters, but it makes use of the two
global variables Line_Number (indicates the line number) and STR_NEXT_LINE
which indicates the string to be scanned and analyzed */

void PARSE_STRING()
{
     int STRING_INDEX = 0;
     int LENGTH_OF_STRING = strlen(STR_NEXT_LINE);
     bool bDeclaring = false;
     bool bAssigning = false;
     bool bDeclaring_Function = false;
     bool bDeclaring_Constant = false;
     bool bDeclaring_Array = false;
     String var_type;
     
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
              if (ERROR_TYPE > 0)
              {
                 if (ERROR_TYPE == 1)
                 {
                      if (bDeclaring && !bAssigning && !bDeclaring_Function && !bDeclaring_Array)
                      {
                         ADD_VARIABLE(TEMP_STRING,var_type);
                      }
                      if (bDeclaring_Array && !IS_RESERVED(TEMP_STRING))
                      {
                         ADD_ARRAY_LIST(TEMP_STRING);
                         bDeclaring_Array = false;
                      }
                      if (isDeclaring(TEMP_STRING))
                      {
                           bDeclaring = true;
                           strcpy(var_type,TEMP_STRING);
                      }
                      else if (isDeclaringFunction(TEMP_STRING))
                      {
                           bDeclaring_Function = true;
                           CLEAR_LOCAL_VARIABLES();
                      }
                      else if (isDeclaringArray(TEMP_STRING))
                      {
                           bDeclaring_Array = true;
                      }
                      if (bDeclaring_Function == true && !IS_RESERVED(TEMP_STRING))
                      {
                           ADD_FUNCTION_LIST(TEMP_STRING);
                           bDeclaring_Function = false;
                      }
                      if (bDeclaring_Constant == true && !IS_RESERVED(TEMP_STRING))
                      {
                           ADD_CONSTANT_LIST(TEMP_STRING);
                        //   printf ("Added Constant to the list: %s\n",TEMP_STRING);
   
                           bDeclaring_Constant = false;
                      }
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
                    //    fprintf (pFile,"ERROR DETECTED at column %d! Unterminated String syntax\n",START_COLUMN_NUMBER);
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
                   //     fprintf (pFile,"Character token \'%s detected at column %d\n",TEMP_STRING,START_COLUMN_NUMBER);
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
                           if (STRING_INDEX+1 < LENGTH_OF_STRING && STR_NEXT_LINE[STRING_INDEX+1] == '/')
                           {
                              STRING_INDEX++;
                              temp[1] = '/';
                              temp[2] = '\0';
                              PUSH(temp);
                           }
                           else
                               PUSH(temp);
                           break;
                           
                      case '+':
                      case '-':
                      case '*':
                      case '$':
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
                          if (strcmp(temp,"=") == 0)
                             bAssigning = true;
                           break;
                      
                      case ';':
                          // NOTIFY_OF_TOKEN(SEMICOLON_TOKEN,CHARACTER_TO_EVALUATE,STRING_INDEX);
                          PUSH(temp);
                       //    PRINT_STACK_CONTENTS();
                           break;
                      
                      case ')':
                           PUSH(temp);
                        //   PRINT_STACK_CONTENTS();
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
                           {
                              HAS_ERROR = true;
                              printf ("ERROR: EXCESS '}'");
                           }
                           break;
                           
                      case ',':
                      case '#':
                           PUSH(temp);
                           if (strcmp(temp,",") == 0)
                              bAssigning = false;
                           else if (strcmp(temp,"#") == 0)
                              bDeclaring_Constant = true;
                           break;
                      
               }
               STRING_INDEX++;
           }
     }
}


/* TRIM_STRING is a function which simulates java.String.trim(); function, which
eliminates all the beginning white spaces, and all excess white spaces, as well as
all the trailing white spaces */
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

/**** This is a BLOOOOOOODY function, simulating a BOTTOM-UP-ALA-BRUTE-FORCED-ART-CARONONGAN-STYLE ****/
bool VALID_EQUATION(int start,int end)
{
     int i = start,n = 1;
     int numParenthesis = 0;
     printf ("start = %d\n",start);
     printf ("end = %d\n",end);
     if (start == end)
        return true;
     while (i <= end)
     {
           puts(Stack[i]);
           if (!strcmp(Stack[i],"op_paren"))
              numParenthesis++;
           else if (!strcmp(Stack[i],"cl_paren"))
              numParenthesis--;
           i++;
     }
     if (numParenthesis != 0)
     {
        printf ("%d\n",numParenthesis);
        printf ("Imbalanced parenthesis\n");
        return false;
     }
     i = start;
     while (i <= end)
     {
           printf ("%d\n",n);
           if (strcmp(Stack[i],"op_paren") == 0 || strcmp(Stack[i],"cl_paren") == 0)
           {
              i++;
              continue;
           }
           fprintf (pFile,"%d\n",n);
           fprintf (pFile,"%s\n",Stack[i]);
           switch (n)
           {
                  case 1:
                       if (strcmp(Stack[i],"optr") == 0)
                       {
                                                   i++;
                                                   n = 2;
                       }
                       else if (strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"INTEGER") == 0 || strcmp(Stack[i],"FLOATING") == 0)
                       {
                                                  i += 2;
                                                  n = 2;
                       }
                       else if (strcmp(Stack[i],"comma") == 0 || strcmp(Stack[i],"sem_col") == 0)
                            return true;
                       else
                           return false;
                       break;
                  case 2:
                       if (i >= STACK_INDEX)
                          return false;
                       if (strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"INTEGER") == 0 || strcmp(Stack[i],"FLOATING") == 0)
                       {
                                                         i++;
                                                         n = 3;
                       }
                       else if (strcmp(Stack[i],"optr") == 0)
                       {
                            i++;
                            n = 4;
                       }
                       else
                           return false;
                       break;
                  case 3:
                       if (i == end)
                          return true;
                       else if (strcmp(Stack[i],"optr") == 0)
                       {
                            i++;
                            n = 2;
                       }
                       else
                           return false;
                       break;
                  case 4:
                       if (i == end)
                          return true;
                       else
                          return false;
           }
     }
}

bool PARSE_INT_TREE()
{
     int n = 1,i = 1;
     bool bAccept = false;
   //  fprintf (pFile,"Entered?");
     int nStart,counter;
     while (!bAccept && i <= STACK_INDEX)
     {
           fprintf(pFile,"%d\n",n);
        //   fprintf (pFile,"Returned!\n");
           switch (n)
           {
                  case 1:
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                           i++;
                           n = 2;
                       }
                       else
                           return false;
                  break;
                  
                  case 2:
                       if (i < STACK_INDEX && strcmp(Stack[i],"sem_col") == 0)
                       {
                        //  fprintf (pFile,"Reached Semicolon\n");
                          i++;
                          n = 4;
                      //    fprintf (pFile,"%d\n",n);
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"comma") == 0)
                       {
                            i++;
                            n = 6;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"ass_val") == 0)
                       {
                            i++;
                            n = 3;
                       }
                       else
                           return false;
                       break;
                  case 3:
                       if ((strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"INTEGER") == 0 && strcmp(Stack[i+1],"optr") == 0))
                       {
                            nStart = i+1;
                            counter = i+1;
                            while (counter < STACK_INDEX && strcmp(Stack[counter],"IDENTIFIER") == 0 || strcmp(Stack[counter],"INTEGER") == 0 || strcmp(Stack[counter],"optr") == 0 || strcmp(Stack[counter],"op_paren") == 0 || strcmp(Stack[counter],"cl_paren") == 0)
                                  counter++;
                            if (counter == STACK_INDEX)
                            {
                               return false;
                            }
                            else
                            {
                                if (VALID_EQUATION(nStart,counter))
                                {
                                     i = counter;
                                     n = 2;
                                }
                                else
                                {
                                    fprintf(pFile,"Returned false here!\n");
                                    return false;
                                }
                            }
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                          i++;
                          n = 2;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"INTEGER") == 0)
                       {
                            i++;
                            n = 5;
                       }
                       else
                           return false;
                       break;
                  case 4:
                      // fprintf (pFile,"%d == %d\n",i,STACK_INDEX);
                       if (i == STACK_INDEX)
                       {
                      //    fprintf (pFile,"True returned!\n");
                          return true;
                       }
                       else
                       {
                     //      fprintf (pFile,"Reached Semicolon when stack not yet cleared\n");
                           return false;
                       }
                       break;
                  case 5:
                       if (i < STACK_INDEX && strcmp(Stack[i],"sem_col") == 0)
                       {
                          i++;
                          n = 4;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"comma") == 0)
                       {
                            i++;
                            n = 6;
                       }
                       else
                       return false;
                       break;
                  case 6:
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                                                         i++;
                                                         n = 2;
                       }
                       else
                           return false;
                       break;
           }
     }
     return false;
}

bool PARSE_FLOAT_TREE()
{
     int n = 1,i = 1;
     bool bAccept = false;
   //  fprintf (pFile,"Entered?");
   int nStart,counter;
     while (!bAccept && i <= STACK_INDEX)
     {
        //   fprintf(pFile,"%d\n",n);
        //   fprintf (pFile,"Returned!\n");
           switch (n)
           {
                  case 1:
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                           i++;
                           n = 2;
                       }
                       else
                           return false;
                  break;
                  
                  case 2:
                       if (i < STACK_INDEX && strcmp(Stack[i],"sem_col") == 0)
                       {
                        //  fprintf (pFile,"Reached Semicolon\n");
                          i++;
                          n = 4;
                      //    fprintf (pFile,"%d\n",n);
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"comma") == 0)
                       {
                            i++;
                            n = 6;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"ass_val") == 0)
                       {
                            i++;
                            n = 3;
                       }
                       else
                           return false;
                       break;
                  case 3:
                      /* if ((strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"INTEGER") == 0 || strcmp(Stack[i],"FLOATING") == 0 && strcmp(Stack[i+1],"optr") == 0))
                       {
                            nStart = i+1;
                            counter = i+1;
                            while (counter < STACK_INDEX && strcmp(Stack[counter],"IDENTIFIER") == 0 || strcmp(Stack[counter],"INTEGER") == 0 || strcmp(Stack[i],"FLOATING") == 0 || strcmp(Stack[counter],"optr") == 0)
                                  counter++;
                            if (counter == STACK_INDEX)
                               return false;
                            else
                            {
                                if (VALID_EQUATION(nStart,counter))
                                {
                                     i = counter;
                                     n = 5;
                                }
                                else
                                    return false;
                            }
                       } */
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                          i++;
                          n = 2;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"INTEGER")== 0)
                       {
                            i++;
                            n = 5;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"FLOATING")== 0)
                       {
                            i++;
                            n = 5;
                       }
                       else
                           return false;
                       break;
                  case 4:
                      // fprintf (pFile,"%d == %d\n",i,STACK_INDEX);
                       if (i == STACK_INDEX)
                       {
                      //    fprintf (pFile,"True returned!\n");
                          return true;
                       }
                       else
                       {
                     //      fprintf (pFile,"Reached Semicolon when stack not yet cleared\n");
                           return false;
                       }
                       break;
                  case 5:
                       if (i < STACK_INDEX && strcmp(Stack[i],"sem_col") == 0)
                       {
                          i++;
                          n = 4;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"comma") == 0)
                       {
                            i++;
                            n = 6;
                       }
                       else
                       return false;
                       break;
                  case 6:
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                                                         i++;
                                                         n = 2;
                       }
                       else
                           return false;
                       break;
           }
     }
     return false;
}

bool PARSE_BOOLEAN_TREE()
{
     int n = 1,i = 1;
     bool bAccept = false;
   //  fprintf (pFile,"Entered?");
     while (!bAccept && i <= STACK_INDEX)
     {
         //  fprintf(pFile,"%d\n",n);
        //   fprintf (pFile,"Returned!\n");
           switch (n)
           {
                  case 1:
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                           i++;
                           n = 2;
                       }
                       else
                           return false;
                  break;
                  
                  case 2:
                       if (i < STACK_INDEX && strcmp(Stack[i],"sem_col") == 0)
                       {
                        //  fprintf (pFile,"Reached Semicolon\n");
                          i++;
                          n = 4;
                      //    fprintf (pFile,"%d\n",n);
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"comma") == 0)
                       {
                            i++;
                            n = 6;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"ass_val") == 0)
                       {
                            i++;
                            n = 3;
                       }
                       else
                           return false;
                       break;
                  case 3:
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                          i++;
                          n = 2;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"bool_sign")== 0)
                       {
                            i++;
                            n = 5;
                       }
                       else
                           return false;
                       break;
                  case 4:
                      // fprintf (pFile,"%d == %d\n",i,STACK_INDEX);
                       if (i == STACK_INDEX)
                       {
                      //    fprintf (pFile,"True returned!\n");
                          return true;
                       }
                       else
                       {
                     //      fprintf (pFile,"Reached Semicolon when stack not yet cleared\n");
                           return false;
                       }
                       break;
                  case 5:
                       if (i < STACK_INDEX && strcmp(Stack[i],"sem_col") == 0)
                       {
                          i++;
                          n = 4;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"comma") == 0)
                       {
                            i++;
                            n = 6;
                       }
                       else
                       return false;
                       break;
                  case 6:
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                                                         i++;
                                                         n = 2;
                       }
                       else
                           return false;
                       break;
           }
     }
     return false;
}

bool PARSE_CHAR_TREE()
{
     int n = 1,i = 1;
     bool bAccept = false;
   //  fprintf (pFile,"Entered?");
     while (!bAccept && i <= STACK_INDEX)
     {
                     printf("%d\n",n);
                     //getch();
      //     fprintf(pFile,"%d\n",n);
        //   fprintf (pFile,"Returned!\n");
           switch (n)
           {
                  case 1:
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                           i++;
                           n = 2;
                       }
                       else
                           return false;
                  break;
                  
                  case 2:
                       if (i < STACK_INDEX && strcmp(Stack[i],"sem_col") == 0)
                       {
                        //  fprintf (pFile,"Reached Semicolon\n");
                          i++;
                          n = 4;
                      //    fprintf (pFile,"%d\n",n);
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"comma") == 0)
                       {
                            i++;
                            n = 6;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"ass_val") == 0)
                       {
                            i++;
                            n = 3;
                       }
                       else
                           return false;
                       break;
                  case 3:
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                          i++;
                          n = 2;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"sing_quote")== 0)
                       {
                            i++;
                            n = 7;
                       }
                       else
                           return false;
                       break;
                  case 7:
                       if (i < STACK_INDEX && (strcmp(Stack[i],"UNDEFINED") == 0 || strcmp(Stack[i],"IDENTIFIER") == 0))
                       {
                          i++;
                          n = 8;
                       }
                       else
                           return false;
                       break;
                  case 8:
                       if (i < STACK_INDEX && strcmp(Stack[i],"sing_quote") == 0)
                       {
                          i++;
                          n = 5;
                       }
                       else
                           return false;
                       break;
                  case 4:
                       if (i == STACK_INDEX)
                       {
                      //    fprintf (pFile,"True returned!\n");
                          return true;
                       }
                       else
                       {
                     //      fprintf (pFile,"Reached Semicolon when stack not yet cleared\n");
                           return false;
                       }
                       break;
                  case 5:
                       if (i < STACK_INDEX && strcmp(Stack[i],"sem_col") == 0)
                       {
                          i++;
                          n = 4;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"comma") == 0)
                       {
                            i++;
                            n = 6;
                       }
                       else
                       return false;
                       break;
                  case 6:
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                                                         i++;
                                                         n = 2;
                       }
                       else
                           return false;
                       break;
           }
     }
     return false;
}

bool PARSE_STRING_TREE()
{
     int n = 1,i = 1;
     bool bAccept = false;
   //  fprintf (pFile,"Entered?");
     while (!bAccept && i <= STACK_INDEX)
     {
      //     fprintf(pFile,"%d\n",n);
        //   fprintf (pFile,"Returned!\n");
           switch (n)
           {
                  case 1:
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                           i++;
                           n = 2;
                       }
                       else
                           return false;
                  break;
                  
                  case 2:
                       if (i < STACK_INDEX && strcmp(Stack[i],"sem_col") == 0)
                       {
                        //  fprintf (pFile,"Reached Semicolon\n");
                          i++;
                          n = 4;
                      //    fprintf (pFile,"%d\n",n);
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"comma") == 0)
                       {
                            i++;
                            n = 6;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"ass_val") == 0)
                       {
                            i++;
                            n = 3;
                       }
                       else
                           return false;
                       break;
                  case 3:
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                          i++;
                          n = 2;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"doub_quote")== 0)
                       {
                            i++;
                            n = 7;
                       }
                       else
                           return false;
                       break;
                  case 7:
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                          i++;
                          n = 8;
                       }
                       else
                           return false;
                       break;
                  case 8:
                       if (i < STACK_INDEX && strcmp(Stack[i],"doub_quote") == 0)
                       {
                          i++;
                          n = 5;
                       }
                       else
                           return false;
                       break;
                  case 4:
                       if (i == STACK_INDEX)
                       {
                      //    fprintf (pFile,"True returned!\n");
                          return true;
                       }
                       else
                       {
                     //      fprintf (pFile,"Reached Semicolon when stack not yet cleared\n");
                           return false;
                       }
                       break;
                  case 5:
                       if (i < STACK_INDEX && strcmp(Stack[i],"sem_col") == 0)
                       {
                          i++;
                          n = 4;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"comma") == 0)
                       {
                            i++;
                            n = 6;
                       }
                       else
                       return false;
                       break;
                  case 6:
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                                                         i++;
                                                         n = 2;
                       }
                       else
                           return false;
                       break;
           }
     }
     return false;
}

bool PARSE_FUNCTION_TREE()
{
     int n = 0,i = 1;
     bool bAccept = false;
   //  fprintf (pFile,"Entered?");
     while (!bAccept && i <= STACK_INDEX)
     {
       //    fprintf(pFile,"%d\n",n);
        //   fprintf (pFile,"Returned!\n");
           switch (n)
           {
                  case 0:
                  {
                       if (i < STACK_INDEX && strcmp(Stack[i],"void_optr") == 0 ||
                            strcmp(Stack[i],"int_optr") == 0 || strcmp(Stack[i],"float_optr") == 0 ||
                            strcmp(Stack[i],"boolean_optr") == 0 || strcmp(Stack[i],"char_optr") == 0 ||
                            strcmp(Stack[i],"string_optr") == 0)
                       {
                            i++;
                            n = 1;
                       }
                       else
                           return false;
                  }
                  break;
                  
                  case 1:
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"UNKNOWN"))
                       {
                           i++;
                           n = 2;
                       }
                       else
                           return false;
                  break;
                  
                  case 2:
                       if (i < STACK_INDEX && strcmp(Stack[i],"op_paren") == 0)
                       {
                          i++;
                          n = 3;
                      }
                       else
                           return false;
                       break;
                  case 3:
                       if (i < STACK_INDEX && strcmp(Stack[i],"cl_paren") == 0)
                       {
                          i++;
                          n = 4;
                       }
                       else if (i < STACK_INDEX &&
                            strcmp(Stack[i],"int_optr") == 0 || strcmp(Stack[i],"float_optr") == 0 ||
                            strcmp(Stack[i],"boolean_optr") == 0 || strcmp(Stack[i],"char_optr") == 0 ||
                            strcmp(Stack[i],"string_optr") == 0)
                       {
                            i++;
                            n = 5;
                       }
                       else
                           return false;
                       break;
                  case 7:
                      if (i < STACK_INDEX &&
                            strcmp(Stack[i],"int_optr") == 0 || strcmp(Stack[i],"float_optr") == 0 ||
                            strcmp(Stack[i],"boolean_optr") == 0 || strcmp(Stack[i],"char_optr") == 0 ||
                            strcmp(Stack[i],"string_optr") == 0)
                       {
                            i++;
                            n = 5;
                       }
                       else
                           return false;
                       break;
                  case 4:
                       if (i == STACK_INDEX)
                       {
                          return true;
                       }
                       else
                       {
                           return false;
                       }
                       break;
                  case 5:
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                                                         i++;
                                                         n = 6;
                       }
                       else
                           return false;
                       break;
                  case 6:
                       if (i < STACK_INDEX && strcmp(Stack[i],"cl_paren") == 0)
                       {
                          i++;
                          n = 4;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"comma") == 0)
                       {
                                                         i++;
                                                         n = 7;
                       }
                       else
                           return false;
                       break;
           }
     }
     return false;
}

bool PARSE_IDENTIFIER_TREE()
{
     int n = 1,i = 1;
     bool bAccept = false;
     while (!bAccept && i <= STACK_INDEX)
     {
          // fprintf (pFile,"%d\n",n);
           switch (n)
           {
                  case 1:
                       if (i < STACK_INDEX && strcmp(Stack[i],"op_paren") == 0)
                       {
                             i++;
                             n = 5;
                       }
                       else
                           return false;
                       break;
                  case 2:
                       break;
                  case 3:
                       break;
                  case 4:
                       break;
                  case 5:
                       if (i < STACK_INDEX && (strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"INTEGER") == 0))
                       {
                             i++;
                             n = 6;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"cl_paren") == 0)
                       {
                            i++;
                            n = 8;
                       }
                       else
                           return false;
                       break;
                  case 6:
                       if (i < STACK_INDEX && strcmp(Stack[i],"comma") == 0)
                       {
                             i++;
                             n = 7;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"cl_paren") == 0)
                       {
                            i++;
                            n = 8;
                       }
                       else
                           return false;
                       break;
                  case 7:
                       if (i < STACK_INDEX && (strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"INTEGER") == 0))
                       {
                             i++;
                             n = 6;
                       }
                       else
                           return false;
                       break;
                  case 8:
                       if (i < STACK_INDEX && strcmp(Stack[i],"sem_col") == 0)
                       {
                             i++;
                             n = 9;
                       }
                       else
                           return false;
                       break;
                  case 9:
                       if (i == STACK_INDEX) 
                          return true;
                       else
                           return false;
                       break;
           }
     }
     return false;
}

bool PARSE_READ_TREE()
{
     int n = 1,i = 1;
     bool bAccept = false;
     while (!bAccept && i <= STACK_INDEX)
     {
           switch (n)
           {
                  case 1:
                       if (i < STACK_INDEX && strcmp(Stack[i],"op_paren") == 0)
                       {
                             i++;
                             n = 2;
                       }
                       else
                           return false;
                       break;
                  case 2:
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"INTEGER") == 0|| strcmp(Stack[i],"FLOATING") == 0)
                       {
                             i++;
                             n = 3;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"cl_paren") == 0)
                       {
                            i++;
                            n = 4;
                       }
                       else
                           return false;
                       break;
                  case 3:
                       if (i < STACK_INDEX && strcmp(Stack[i],"cl_paren") == 0)
                       {
                            i++;
                            n = 4;
                       }
                       else
                           return false;
                       break;
                  case 4:
                       if (i < STACK_INDEX && strcmp(Stack[i],"sem_col") == 0)
                       {
                             i++;
                             n = 5;
                       }
                       else
                           return false;
                       break;
                  case 5:
                       if (i == STACK_INDEX)
                          return true;
                       else
                           return false;
                       break;
           }
     }
}

bool PARSE_ASSIGN_FUNCTION_TREE()
{
     int n = 1,i = 1;
     bool bAccept = false;
     while (!bAccept && i <= STACK_INDEX)
     {
           switch (n)
           {
                  case 1:
                       if (i < STACK_INDEX && strcmp(Stack[i],"op_paren") == 0)
                       {
                             i++;
                             n = 2;
                       }
                       else
                           return false;
                       break;
                  case 2:
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                             i++;
                             n = 3;
                       }
                       else
                           return false;
                       break;
                  case 3:
                       if (i < STACK_INDEX && strcmp(Stack[i],"cl_paren") == 0)
                       {
                             i++;
                             n = 4;
                       }
                       else
                           return false;
                       break;
                  case 4:
                       if (i < STACK_INDEX && strcmp(Stack[i],"ass_val") == 0)
                       {
                             i++;
                             n = 5;
                       }
                       else
                           return false;
                       break;
                  case 5:
                       if (i < STACK_INDEX && strcmp(Stack[i],"FUNCTION_CALL") == 0)
                       {
                             i++;
                             n = 6;
                       }
                       else
                           return false;
                       break;
                  case 6:
                       if (i < STACK_INDEX && strcmp(Stack[i],"op_paren") == 0)
                       {
                             i++;
                             n = 7;
                       }
                       else
                           return false;
                       break;
                  case 7:
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"INTEGER") == 0|| strcmp(Stack[i],"FLOATING") == 0)
                       {
                             i++;
                             n = 8;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"cl_paren") == 0)
                       {
                            i++;
                            n = 9;
                       }
                       else
                           return false;
                       break;
                  case 8:
                       if (i < STACK_INDEX && strcmp(Stack[i],"cl_paren") == 0)
                       {
                            i++;
                            n = 9;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"comma") == 0)
                       {
                            i++;
                            n = 7;
                       }
                       else
                           return false;
                       break;
                  case 9:
                       if (i < STACK_INDEX && strcmp(Stack[i],"sem_col") == 0)
                       {
                             i++;
                             n = 10;
                       }
                       else
                           return false;
                       break;
                  case 10:
                       if (i == STACK_INDEX)
                          return true;
                       else
                           return false;
                       break;
                  
           }
     }
}

bool PARSE_WRITE_TREE()
{
     int n = 1,i = 1;
     bool bAccept = false;
     while (!bAccept && i <= STACK_INDEX)
     {
        //   printf ("%d\n",n);
        //   printf ("%s\n",Stack[i]);
        //   fprintf (pFile,"%d\n",n);
           switch (n)
           {
                  case 1: 
                       if (i < STACK_INDEX && strcmp(Stack[i],"op_paren") == 0)
                       {
                             i++;
                             n = 2;
                       }
                       else
                           return false;
                       break;
                  case 2:                        
                       if (i < STACK_INDEX && strcmp(Stack[i],"doub_quote") == 0)
                       {
                             i++;
                             n = 3;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                             i++;
                             n = 6;
                       }
                       else
                           return false;
                       break;
                  case 3: 
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"UNDEFINED") == 0 || strcmp(Stack[i],"CONST") == 0)
                       {
                             i++;
                             n = 4;
                       }
                       else
                           return false;
                       break;
                  case 4: 
                       if (i < STACK_INDEX && strcmp(Stack[i],"doub_quote") == 0)
                       {
                             i++;
                             n = 5;
                       }
                       else
                           return false;
                       break;
                  case 5:                     
                       if (i < STACK_INDEX && strcmp(Stack[i],"cl_paren") == 0)
                       {
                             i++;
                             n = 8;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"optr") == 0)
                       {
                            i++;
                            n = 7;
                       }
                       else
                           return false;
                       break;
                  case 6: 
                       if (i < STACK_INDEX && strcmp(Stack[i],"optr") == 0)
                       {
                             i++;
                             n = 7;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"cl_paren") == 0)
                       {
                             i++;
                             n = 8;
                       }
                       else
                           return false;
                       break;
                  case 7: 
                       if (i < STACK_INDEX && strcmp(Stack[i],"doub_quote") == 0)
                       {
                             i++;
                             n = 3;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                             i++;
                             n = 6;
                       }
                       else
                           return false;
                       break;
                  case 8: 
                       if (i < STACK_INDEX && strcmp(Stack[i],"sem_col") == 0)
                       {
                             i++;
                             n = 9;
                       }
                       else
                           return false;
                       break;
                  case 9: 
                       if (i == STACK_INDEX)
                          return true;
                       else
                           return false;
                       break;
           }
     }
}

bool PARSE_DEFINE_TREE()
{
     int n = 1,i = 1;
     bool bAccept = false;
     while (!bAccept && i <= STACK_INDEX)
     {
         //  fprintf (pFile,"%d\n",n);
           switch (n)
           {
                  case 1: 
                       if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                             i++;
                             n = 2;
                       }
                       else
                           return false;
                  break;
                  case 2:                        
                       if (i < STACK_INDEX && strcmp(Stack[i],"as") == 0)
                       {
                             i++;
                             n = 3;
                       }
                       else
                           return false;
                       break;
                  case 3: 
                       if (i < STACK_INDEX && strcmp(Stack[i],"doub_quote") == 0)
                       {
                             i++;
                             n = 6;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"INTEGER") == 0 || strcmp(Stack[i],"FLOATING") == 0)
                       {
                             i++;
                             n = 4;
                       }
                       else
                           return false;
                       break;
                  case 4: 
                       if (i < STACK_INDEX && strcmp(Stack[i],"sem_col") == 0)
                       {
                             i++;
                             n = 5;
                       }
                       else
                           return false;
                       break;
                  case 5:                     
                       if (i == STACK_INDEX)
                          return true;
                       else
                           return false;
                       break;
                  case 6: 
                       if (i < STACK_INDEX && strcmp(Stack[i],"doub_quote") == 0)
                       {
                             i++;
                             n = 4;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                             i++;
                             n = 7;
                       }
                       else
                           return false;
                       break;
                  case 7: 
                       if (i < STACK_INDEX && strcmp(Stack[i],"doub_quote") == 0)
                       {
                             i++;
                             n = 4;
                       }
                       else
                           return false;
                       break;
           }
     }
}

bool VALID_BOOL_EXPR (int start,int end)
{
     int n = 1,i = start;
     
     printf ("Evaluating: ");
     for (i = start;i <= end;i++)
         printf ("%s ",Stack[i]);
     printf ("\n");
     i = start;
     
     int start_index = start,end_index = start;
     while (i <= end)
     {
           printf ("evaluating boolean expression\n");
           while (i <= end && strcmp("rel_optr",Stack[i]) != 0)
           {
                 i++;
                 end_index++;
           }
           end_index--;
           for (int j = start_index;j <= end_index;j++)
               printf ("%s ",Stack[j]);
           printf ("\n");
           if (VALID_EQUATION(start_index,end_index))
           {
                 start_index = end_index + 1;
                 if (strcmp(Stack[start_index],"rel_optr") == 0)
                    start_index++;
                 end_index = start_index;
                 i++;
                 if (i <= end && strcmp("rel_optr",Stack[i]) == 0)
                    i++;
           }
           else
           {
               printf ("Invalid equation returned \n");
               return false;
           }
     }
     if (i >= end)
        return true;
     else
     {
        printf ("False returned here \n");
        return false;
     }
   //  fprintf (pFile,"%s\n",Stack[i]);
   /*
    if (strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"bool_sign") == 0)
     {
         i++;
         if (i == end)
            return true;
         while (i <= end)
         {
        //       fprintf (pFile,"%d in valid\n");
               
               switch (n)
               {
                      case 1:
                           if (strcmp(Stack[i],"rel_optr") == 0)
                           {
                                                           i++;
                                                           n = 2;
                           }
                           else
                               return false;
                           break;
                      case 2:
                           if (strcmp(Stack[i],"INTEGER") == 0 || strcmp(Stack[i],"FLOATING") == 0 || strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"bool_sign") == 0)
                           {
                                                          i++;
                                                          n = 3;
                           }
                           else
                               return false;
                           break;
                      case 3:
                           if (i == end)
                              return true;
                           else
                              return false;
                           break;
                      
               }
         }
     } */
}

bool PARSE_IF_TREE()
{
     int i = 1,n = 1;
     
     int nEnd,nStart,counter,numParenthesis;
     
     numParenthesis = 0;
     
     while (i < STACK_INDEX)
     {
           puts (Stack[i]);
           if (i != STACK_INDEX && strcmp(Stack[i],"cl_paren") == 0)
              numParenthesis--;
           else if (strcmp(Stack[i],"op_paren") == 0)
              numParenthesis++;
           i++;
           printf ("%d\n",numParenthesis);
     }
     if (numParenthesis != 0)
     {
           fprintf(pFile,"Imbalanced parenthesis\n");
           printf ("PARSE_IF_TREE returned: ");
           printf ("Imbalanced parenthesis!\n");
           return false;
     }
     
     i = 1;
     while (i <= STACK_INDEX)
     {
        //   fprintf (pFile,"%d\n",n);
          printf ("%d\n",n);
           switch (n)
           {
                  case 1:
                       if (strcmp(Stack[i],"op_paren") == 0)
                       {
                           i++;
                           n = 2;
                       }
                       else
                           return false;
                       break;
                  case 2:
                       nStart = i;
                       counter = nStart;
                       numParenthesis = 1;
                       while (counter < STACK_INDEX && strcmp("comp_log_or",Stack[counter]) != 0 && strcmp("comp_log_amp",Stack[counter]) != 0)
                       {
                              counter++;
                       }
                       if (counter == STACK_INDEX && strcmp(Stack[counter-1],"cl_paren") != 0)
                          return false;
                       else
                       {
                           nEnd = counter;
                           printf ("Original nEnd = %d\n",nEnd);
                           if (counter == STACK_INDEX)
                              nEnd-=2;
                           else if (strcmp(Stack[nEnd],"comp_log_or") == 0 || strcmp(Stack[nEnd],"comp_log_amp") == 0)
                              nEnd--;
                    //       fprintf (pFile,"Evaluating\n");
                      //     fprintf (pFile,"Start at %d, end at %d\n",nStart,nEnd);
                           if (VALID_BOOL_EXPR(nStart,nEnd))
                           {
                              i = nEnd+1;
                              nEnd = i+1;
                              n = 3;
                           }
                           else
                               return false;
                       }
                       break;
                 case 3:
                      if (strcmp("cl_paren",Stack[i]) == 0)
                      {
                           i++;
                           n = 4;
                      }
                      else if (strcmp("comp_log_amp",Stack[i]) == 0)
                      {
                           i++;
                           n = 5;
                      }
                      else if (strcmp("comp_log_or",Stack[i]) == 0)
                      {
                           i++;
                           n = 6;
                      }
                      else
                          return false;
                      break;
                case 4:
                       if (i == STACK_INDEX)
                          return true;
                         else
                         return false;
                        break;  
                case 5:
                     if (strcmp("comp_log_amp",Stack[i]) == 0)
                     {
                          i++;
                          n = 2;
                     }
                     else
                         return false;
                     break;    
                case 6:
                     if (strcmp("comp_log_or",Stack[i]) == 0)
                     {
                          i++;
                          n = 2;
                     }
                     else
                         return false;
                     break;    
           }
     }
}

bool PARSE_DO_WHILE_TREE()
{
	          int i = 1,n = 1;
     
     int nEnd,nStart,counter,numParenthesis;
     
     numParenthesis = 0;
     
     while (i < STACK_INDEX)
     {
           puts (Stack[i]);
           if (i != STACK_INDEX && strcmp(Stack[i],"cl_paren") == 0)
              numParenthesis--;
           else if (strcmp(Stack[i],"op_paren") == 0)
              numParenthesis++;
           i++;
           printf ("%d\n",numParenthesis);
     }
     if (numParenthesis != 0)
     {
           fprintf(pFile,"Imbalanced parenthesis\n");
           printf ("PARSE_DO_WHILE_TREE returned: ");
           printf ("Imbalanced parenthesis!\n");
           return false;
     }
     
     i = 1;
     while (i <= STACK_INDEX)
     {
          printf ("%d\n",n);
           switch (n)
           {
                  case 1:
                       if (strcmp(Stack[i],"op_paren") == 0)
                       {
                           i++;
                           n = 2;
                       }
                       else
                           return false;
                       break;
                  case 2:
                       nStart = i;
                       counter = nStart;
                       numParenthesis = 1;
                       while (counter < STACK_INDEX && strcmp("comp_log_or",Stack[counter]) != 0 && strcmp("comp_log_amp",Stack[counter]) != 0)
                       {
                              counter++;
                       }
                       if (counter == STACK_INDEX && strcmp(Stack[counter-1],"cl_paren") != 0)
                          return false;
                       else
                       {
                           nEnd = counter;
                           printf ("Original nEnd = %d\n",nEnd);
                           if (counter == STACK_INDEX)
                              nEnd-=2;
                           else if (strcmp(Stack[nEnd],"comp_log_or") == 0 || strcmp(Stack[nEnd],"comp_log_amp") == 0)
                              nEnd--;
                           if (VALID_BOOL_EXPR(nStart,nEnd))
                           {
                              i = nEnd+1;
                              nEnd = i+1;
                              n = 3;
                           }
                           else
                               return false;
                       }
                       break;
                 case 3:
                      if (strcmp("cl_paren",Stack[i]) == 0)
                      {
                           i++;
                           n = 4;
                      }
                      else if (strcmp("comp_log_amp",Stack[i]) == 0)
                      {
                           i++;
                           n = 5;
                      }
                      else if (strcmp("comp_log_or",Stack[i]) == 0)
                      {
                           i++;
                           n = 6;
                      }
                      else
                          return false;
                      break;
                case 4:
                       if (i == STACK_INDEX)
                          return true;
                         else
                         return false;
                        break;  
                case 5:
                     if (strcmp("comp_log_amp",Stack[i]) == 0)
                     {
                          i++;
                          n = 2;
                     }
                     else
                         return false;
                     break;    
                case 6:
                     if (strcmp("comp_log_or",Stack[i]) == 0)
                     {
                          i++;
                          n = 2;
                     }
                     else
                         return false;
                     break;    
           }
     }
}

bool PARSE_WHILE_TREE()
{
          int i = 1,n = 1;
     
     int nEnd,nStart,counter,numParenthesis;
     
     numParenthesis = 0;
     
     while (i < STACK_INDEX)
     {
           puts (Stack[i]);
           if (i != STACK_INDEX && strcmp(Stack[i],"cl_paren") == 0)
              numParenthesis--;
           else if (strcmp(Stack[i],"op_paren") == 0)
              numParenthesis++;
           i++;
           printf ("%d\n",numParenthesis);
     }
     if (numParenthesis != 0)
     {
           fprintf(pFile,"Imbalanced parenthesis\n");
           printf ("PARSE_IF_TREE returned: ");
           printf ("Imbalanced parenthesis!\n");
           return false;
     }
     
     i = 1;
     while (i <= STACK_INDEX)
     {
        //   fprintf (pFile,"%d\n",n);
          printf ("%d\n",n);
           switch (n)
           {
                  case 1:
                       if (strcmp(Stack[i],"op_paren") == 0)
                       {
                           i++;
                           n = 2;
                       }
                       else
                           return false;
                       break;
                  case 2:
                       nStart = i;
                       counter = nStart;
                       numParenthesis = 1;
                       while (counter < STACK_INDEX && strcmp("comp_log_or",Stack[counter]) != 0 && strcmp("comp_log_amp",Stack[counter]) != 0)
                       {
                              counter++;
                       }
                       if (counter == STACK_INDEX && strcmp(Stack[counter-1],"cl_paren") != 0)
                          return false;
                       else
                       {
                           nEnd = counter;
                           printf ("Original nEnd = %d\n",nEnd);
                           if (counter == STACK_INDEX)
                              nEnd-=2;
                           else if (strcmp(Stack[nEnd],"comp_log_or") == 0 || strcmp(Stack[nEnd],"comp_log_amp") == 0)
                              nEnd--;
                    //       fprintf (pFile,"Evaluating\n");
                      //     fprintf (pFile,"Start at %d, end at %d\n",nStart,nEnd);
                           if (VALID_BOOL_EXPR(nStart,nEnd))
                           {
                              i = nEnd+1;
                              nEnd = i+1;
                              n = 3;
                           }
                           else
                               return false;
                       }
                       break;
                 case 3:
                      if (strcmp("cl_paren",Stack[i]) == 0)
                      {
                           i++;
                           n = 4;
                      }
                      else if (strcmp("comp_log_amp",Stack[i]) == 0)
                      {
                           i++;
                           n = 5;
                      }
                      else if (strcmp("comp_log_or",Stack[i]) == 0)
                      {
                           i++;
                           n = 6;
                      }
                      else
                          return false;
                      break;
                case 4:
                       if (i == STACK_INDEX)
                          return true;
                         else
                         return false;
                        break;  
                case 5:
                     if (strcmp("comp_log_amp",Stack[i]) == 0)
                     {
                          i++;
                          n = 2;
                     }
                     else
                         return false;
                     break;    
                case 6:
                     if (strcmp("comp_log_or",Stack[i]) == 0)
                     {
                          i++;
                          n = 2;
                     }
                     else
                         return false;
                     break;    
           }
     }
}

bool PARSE_FOR_TREE()
{
     int i = 1,n = 1;
     
     int nEnd,nStart,counter;
     while (i <= STACK_INDEX)
     {
        printf ("%d\n",n);
		switch (n)
		{
			case 1:
				if (strcmp(Stack[i],"op_paren") == 0)
				{
					i++;
					n = 2;
				}
				else
					return false;
				break;
			case 2:
				if (strcmp(Stack[i],"IDENTIFIER") == 0)
				{
					i++;
					n = 3;
				}
				else
					return false;
				break;
			case 3:
				if (strcmp(Stack[i],"ass_val") == 0)
				{
					i++;
					n = 4;
				}
				else
					return false;
				break;
			case 4:
				if ((strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"INTEGER") == 0 || strcmp(Stack[i],"FLOATING")  == 0) && strcmp(Stack[i+1],"optr") == 0)
                {
					nStart = i+1;
					counter = i+1;
					while (counter < STACK_INDEX && strcmp(Stack[counter],"IDENTIFIER") == 0 || strcmp(Stack[counter],"INTEGER") == 0 || strcmp(Stack[counter],"optr") == 0 || strcmp(Stack[counter],"op_paren") == 0 || strcmp(Stack[counter],"cl_paren") == 0)
						  counter++;
					if (counter == STACK_INDEX)
					   return false;
					else
					{
						if (VALID_EQUATION(nStart,counter))
						{
							 i = counter;
							 n = 5;
						}
						else
							return false;
					}
               }
			   else if (strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"INTEGER") == 0)
               {
                    i++;
                    n = 5;
               }
			   else
					return false;
				break;
			case 5:
				if (strcmp(Stack[i],"sem_col") == 0)
				{
					i++;
					n = 6;
				}
				else
					return false;
				break;
			case 6:
				if (strcmp(Stack[i],"test_optr") == 0)
				{
					i++;
					n = 7;
				}
				else
					return false;
				break;
			case 7:
				    nStart = i;
                    counter = nStart;
                 //   numParenthesis = 1;
                    while (counter < STACK_INDEX && strcmp(Stack[counter],"sem_col") != 0)
                    {
                        counter++;
                    }
				   if (counter == STACK_INDEX)
				   {
                      printf ("Here?\n");
					  return false;
                   }
				   else
				   {
					   nEnd = counter;
					   printf ("Original nEnd = %d\n",nEnd);
					   if (counter == STACK_INDEX)
						  nEnd-=2;
					   else if (strcmp(Stack[nEnd],"sem_col") == 0)
						  nEnd--;
					   if (VALID_BOOL_EXPR(nStart,nEnd))
					   {
						  i = nEnd+1;
						  nEnd = i+1;
						  n = 62;
					   }
					   else
					   {
                           printf ("Returned false here\n");
						   return false;
                       }
				   }
				   break;
            case 62:
                    if (strcmp(Stack[i],"sem_col") == 0)
                    {
                                                   i++;
                                                   n = 8;
                    }
                    else
                        return false;
                    break;
			case 8:
					if (strcmp(Stack[i],"then_optr") == 0)
					{
						i++;
						n = 9;
					}
					else
						return false;
				break;
			case 9:
					if (strcmp(Stack[i],"IDENTIFIER") == 0)
					{
						i++;
						n = 10;
					}
					else
						return false;
					break;
			case 10:
					if (strcmp(Stack[i],"ass_val") == 0)
					{
						i++;
						n = 11;
					}
					else
					{
                        printf ("ERROR WENT HERE?\n");
                        // getch();
    	                return false;
                    }
					break;
			case 11:
				if ((strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"INTEGER") == 0 || strcmp(Stack[i],"FLOATING")  == 0) && strcmp(Stack[i+1],"optr") == 0)
                {
					nStart = i+1;
					counter = i+1;
					while (counter < STACK_INDEX && strcmp(Stack[counter],"IDENTIFIER") == 0 || strcmp(Stack[counter],"INTEGER") == 0 || strcmp(Stack[counter],"optr") == 0 || strcmp(Stack[counter],"op_paren") == 0 || strcmp(Stack[counter],"cl_paren") == 0)
						  counter++;
					if (counter == STACK_INDEX)
					{
                       printf ("ERROR HERE??\n");
                    //   getch();         
					   return false;
                   }
					else
					{
						if (VALID_EQUATION(nStart,counter))
						{
							 i = counter;
							 n = 14;
						}
						else
						{
                            printf ("ERROR HERE??? 12\n");
                        //    getch();
							return false;
                        }
					}
               }
			   else if (strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"INTEGER") == 0)
               {
                    i++;
                    n = 14;
               }
			   else
					return false;
				break;
			case 12:
				if (strcmp(Stack[i],"cl_paren") == 0)
				{
					i++;
					n = 13;
				}
				else
					return false;
				break;
			case 13:
				if (i == STACK_INDEX)
					return true;
				else
					return false;
				break;
		    case 14:
                 if (strcmp(Stack[i],"sem_col") == 0)
                 {
                                                i++;
                                                n = 12;
                 }
                 else
                     return false;
		}
	//	getch();
     }
}

bool PARSE_ASSIGN_TREE()
{
     int i = 2,n = 1;
     
     int nEnd,nStart,counter;
     while (i <= STACK_INDEX)
     {
       //    fprintf (pFile,"%d\n",n);
           switch (n)
           {
                  case 1:
                       if ((strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"INTEGER") == 0 || strcmp(Stack[i],"FLOATING") == 0 || strcmp(Stack[i],"bool_sign") == 0) && strcmp(Stack[i+1],"optr") == 0)
                       {
                            nStart = i+1;
                            counter = i+1;
                            while (counter < STACK_INDEX && strcmp(Stack[counter],"IDENTIFIER") == 0 || strcmp(Stack[counter],"INTEGER") == 0 || strcmp(Stack[counter],"FLOATING") == 0 || strcmp(Stack[counter],"bool_sign") == 0 || strcmp(Stack[counter],"optr") == 0 || strcmp(Stack[counter],"op_paren") == 0 || strcmp(Stack[counter],"cl_paren") == 0)
                                  counter++;
                            if (counter == STACK_INDEX)
                               return false;
                            else
                            {
                                if (VALID_EQUATION(nStart,counter))
                                {
                                     i = counter;
                                     n = 3;
                                }
                                else
                                    return false;
                            }
                       }
                       else if (strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"INTEGER") == 0 || strcmp(Stack[i],"FLOATING") == 0 || strcmp(Stack[i],"bool_sign") == 0)
                       {
                           i++;
                           n = 3;
                       }
                       else
                           return false;
                       break;
                  case 2:
                       if (i == STACK_INDEX)
                          return true;
                       else
                           return false;
                       break;
                  case 3:
                       if (strcmp(Stack[i],"sem_col") == 0)
                       { 
                         i++;
                         n = 2;
                       }
                       else
                           return false;
                       break;
           }
     }
}

bool PARSE_ARRAY_TREE()
{
	int i = 1,n = 1;
	
	while (i <= STACK_INDEX)
	{
          printf ("%d\n",n);
		switch (n)
		{
			case 1:
				if (strcmp(Stack[i],"int_optr") == 0 || strcmp(Stack[i],"float_optr") == 0 || strcmp(Stack[i],"bool_optr") == 0 || strcmp(Stack[i],"char_optr") == 0 || strcmp(Stack[i],"string_optr") == 0)
				{
					i++;
					n = 2;
				}
				else
					return false;
				break;
			case 2:
				if (strcmp(Stack[i],"array_var") == 0)
				{
					i++;
					n = 3;
				}
				else
					return false;
				break;
			case 3:
				if (strcmp(Stack[i],"op_brack") == 0)
				{
					i++;
					n = 4;
				}
				else
					return false;
				break;
			case 4:
				if (strcmp(Stack[i],"INTEGER") == 0)
				{
					i++;
					n = 5;
				}
				else
					return false;
				break;
			case 5:
                 if (strcmp(Stack[i],"cl_brack") == 0)
                 {
                    i++;
                    n = 6;
                 }
                 else
                     return false;
                 break;
			case 6:
				if (strcmp(Stack[i],"sem_col") == 0)
				{
					i++;
					n = 7;
				}
				else
					return false;
				break;
			case 7:
				if (i == STACK_INDEX)
					return true;
				else
					return false;
				break;
		}
	}
	return false;
}

bool PARSE_SET_ARRAY_TREE()
{
     int i = 1,n = 1;
     
     int nStart,counter;
     while (i <= STACK_INDEX)
     {
          // printf ("%d\n",n);
        //   getch();
           switch (n)
           {
               case 1:
                    if (strcmp(Stack[i],"op_paren") == 0)
                    {
                       i++;
                       n = 2;
                    }
                    else
                        return false;
                    break;
               case 2:
                    if (strcmp(Stack[i],"array_var") == 0)
                    {
                       i++;
                       n = 3;
                    }
                    else
                        return false;
                    break;
               case 3:
                    if (strcmp(Stack[i],"op_brack") == 0)
                    {
                       i++;
                       n = 4;
                    }
                    else
                        return false;
                    break;
               case 4:
                       if ((strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"INTEGER") == 0 && strcmp(Stack[i+1],"optr") == 0))
                       {
                            nStart = i+1;
                            counter = i+1;
                            while (counter < STACK_INDEX && strcmp(Stack[counter],"IDENTIFIER") == 0 || strcmp(Stack[counter],"INTEGER") == 0 || strcmp(Stack[counter],"optr") == 0 || strcmp(Stack[counter],"op_paren") == 0 || strcmp(Stack[counter],"cl_paren") == 0)
                                  counter++;
                            if (counter == STACK_INDEX)
                            {
                               return false;
                            }
                            else
                            {
                                if (VALID_EQUATION(nStart,counter))
                                {
                                     i = counter;
                                     n = 5;
                                }
                                else
                                {
                                    fprintf(pFile,"Returned false here!\n");
                                    return false;
                                }
                            }
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                          i++;
                          n = 5;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"INTEGER") == 0)
                       {
                            i++;
                            n = 5;
                       }
                       else
                           return false;
                       break;
               case 5:
                    if (strcmp(Stack[i],"cl_brack") == 0)
                    {
                      i++;
                      n = 6;
                    }
                    else
                        return false;
                    break;
               case 6:
                    if (strcmp(Stack[i],"comma") == 0)
                    {
                      i++;
                      n = 7;
                    }
                    else
                        return false;
                    break;
               case 7:
                    if (strcmp(Stack[i],"IDENTIFIER") == 0)
                    {
                      i++;
                      n = 8;
                    }
                    else
                      return false;
                    break;
               case 8:
                    if (strcmp(Stack[i],"cl_paren") == 0)
                    {
                      i++;
                      n = 9;
                    }
                    else
                        return false;
                    break;
               case 9:
                    if (strcmp(Stack[i],"sem_col") == 0)
                    {
                      i++;
                      n =10;
                    }
                    else
                        return false;
                    break;
               case 10:
                    if (i == STACK_INDEX)
                       return true;
                    else
                        return false;
                    break;
                    
           }
     }
     
     return false;
}

bool PARSE_GET_ARRAY_TREE()
{
     int i = 1,n = 1;
     
     int nStart,counter;
     while (i <= STACK_INDEX)
     {
           switch (n)
           {
               case 1:
                    if (strcmp(Stack[i],"op_paren") == 0)
                    {
                       i++;
                       n = 2;
                    }
                    else
                        return false;
                    break;
               case 2:
                    if (strcmp(Stack[i],"array_var") == 0)
                    {
                       i++;
                       n = 3;
                    }
                    else
                        return false;
                    break;
               case 3:
                    if (strcmp(Stack[i],"op_brack") == 0)
                    {
                       i++;
                       n = 4;
                    }
                    else
                        return false;
                    break;
               case 4:
                      if ((strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"INTEGER") == 0 && strcmp(Stack[i+1],"optr") == 0))
                       {
                            nStart = i+1;
                            counter = i+1;
                            while (counter < STACK_INDEX && strcmp(Stack[counter],"IDENTIFIER") == 0 || strcmp(Stack[counter],"INTEGER") == 0 || strcmp(Stack[counter],"optr") == 0 || strcmp(Stack[counter],"op_paren") == 0 || strcmp(Stack[counter],"cl_paren") == 0)
                                  counter++;
                            if (counter == STACK_INDEX)
                            {
                               return false;
                            }
                            else
                            {
                                if (VALID_EQUATION(nStart,counter))
                                {
                                     i = counter;
                                     n = 5;
                                }
                                else
                                {
                                    fprintf(pFile,"Returned false here!\n");
                                    return false;
                                }
                            }
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                          i++;
                          n = 5;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"INTEGER") == 0)
                       {
                            i++;
                            n = 5;
                       }
                       else
                           return false;
                       break;
               case 5:
                    if (strcmp(Stack[i],"cl_brack") == 0)
                    {
                      i++;
                      n = 6;
                    }
                    else
                        return false;
                    break;
               case 6:
                    if (strcmp(Stack[i],"comma") == 0)
                    {
                      i++;
                      n = 7;
                    }
                    else
                        return false;
                    break;
               case 7:
                    if (strcmp(Stack[i],"IDENTIFIER") == 0)
                    {
                      i++;
                      n = 8;
                    }
                    else
                      return false;
                    break;
               case 8:
                    if (strcmp(Stack[i],"cl_paren") == 0)
                    {
                      i++;
                      n = 9;
                    }
                    else
                        return false;
                    break;
               case 9:
                    if (strcmp(Stack[i],"sem_col") == 0)
                    {
                      i++;
                      n =10;
                    }
                    else
                        return false;
                    break;
               case 10:
                    if (i == STACK_INDEX)
                       return true;
                    else
                        return false;
                    break;
                    
           }
     }
     
     return false;
}

bool PARSE_RETURN_TREE()
{
     int i = 1,n = 1;
     
     int nStart,counter;
     while (i <= STACK_INDEX)
     {
           printf ("%d\n",n);
        //   getch();
           switch (n)
           {
                  case 1:
                       if ((strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"INTEGER") == 0 || strcmp(Stack[i],"FLOATING") == 0 || strcmp(Stack[i],"bool_sign") == 0) && strcmp(Stack[i+1],"optr") == 0)
                       {
                            nStart = i+1;
                            counter = i+1;
                            while (counter < STACK_INDEX && strcmp(Stack[counter],"IDENTIFIER") == 0 || strcmp(Stack[counter],"INTEGER") == 0 || strcmp(Stack[counter],"FLOATING") == 0 || strcmp(Stack[counter],"bool_sign") == 0 || strcmp(Stack[counter],"optr") == 0 || strcmp(Stack[counter],"op_paren") == 0 || strcmp(Stack[counter],"cl_paren") == 0)
                                  counter++;
                            if (counter == STACK_INDEX)
                            {
                               printf ("Unblemished\n");
                            //   getch();
                               return false;
                            }
                            else
                            {
                                if (VALID_EQUATION(nStart,counter))
                                {
                                     i = counter;
                                     n = 2;
                                }
                                else
                                {
                                    printf ("Here!\n");
                             //       getch();
                                    return false;
                                }
                            }
                       }
                       else if (strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"INTEGER") == 0 || strcmp(Stack[i],"FLOATING") == 0 || strcmp(Stack[i],"bool_sign") == 0)
                       {
                           i++;
                           n = 2;
                       }
                       else
                           return false;
                       break;
                  case 2:
                       if (strcmp(Stack[i],"sem_col") == 0)
                       {
                                                      i++;
                                                      n = 3;
                       }
                       else
                           return false;
                       break;
                  case 3:
                       if (i == STACK_INDEX)
                          return true;
                       else
                           return false;
                       break;
                  case 4:
                       if ((strcmp(Stack[i],"IDENTIFIER") == 0 || strcmp(Stack[i],"INTEGER") == 0 && strcmp(Stack[i+1],"optr") == 0))
                       {
                            nStart = i+1;
                            counter = i+1;
                            while (counter < STACK_INDEX && strcmp(Stack[counter],"IDENTIFIER") == 0 || strcmp(Stack[counter],"INTEGER") == 0 || strcmp(Stack[counter],"optr") == 0 || strcmp(Stack[counter],"op_paren") == 0 || strcmp(Stack[counter],"cl_paren") == 0)
                                  counter++;
                            if (counter == STACK_INDEX)
                            {
                               return false;
                            }
                            else
                            {
                                if (VALID_EQUATION(nStart,counter))
                                {
                                     i = counter;
                                     n = 5;
                                }
                                else
                                {
                                    fprintf(pFile,"Returned false here!\n");
                                    return false;
                                }
                            }
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"IDENTIFIER") == 0)
                       {
                          i++;
                          n = 5;
                       }
                       else if (i < STACK_INDEX && strcmp(Stack[i],"INTEGER") == 0)
                       {
                            i++;
                            n = 5;
                       }
                       else
                           return false;
                       break;
                  case 5:
                       if (i < STACK_INDEX && strcmp(Stack[i],"sem_col") == 0)
                       {
                             i++;
                             n = 6;
                       }
                       else
                           return false;
                       break;
                  case 6:
                       if (i == STACK_INDEX)
                          return true;
                       else
                           return false;
                       break;
           }
        //   getch();
     }
}

void EVALUATE_STACK()
{
     if (STACK_INDEX == 0)
       return;
     
     fprintf(pFile,STR_NEXT_LINE);
    // fprintf(pFile,"\n");
     /* Main */
     if (strcmp(Stack[0],"main_optr") == 0 && strcmp(Stack[1],"op_paren") == 0 && strcmp(Stack[2],"cl_paren") == 0 && STACK_INDEX == 3)
     {
         fprintf (pFile,"Main function declaration - ACCEPT");
         printf ("ACCEPT\n");
         return;
     }
     else if (strcmp(Stack[0],"main_optr") == 0 && strcmp(Stack[1],"op_paren") == 0 && strcmp(Stack[2],"cl_paren") == 0 && STACK_INDEX != 3)
     {
         HAS_ERROR = true;
         fprintf (pFile,"Excess tokens to achieve Main function declaration - ERROR");
         printf ("%s\n",STR_NEXT_LINE);
         printf ("ERROR\n");
         fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
         fprintf (pErrorFile,"Message: Excess tokens to achieve Main function declaration\n");
         fprintf (pErrorFile,"\n");
         num_errors++;
         return;
     }
     
     if (strcmp(Stack[0],"assign_optr") == 0)
     {
         if (PARSE_ASSIGN_FUNCTION_TREE() == true)
         {
             fprintf (pFile,"Function assign variable(s) declaration - ACCEPT");
             printf ("ACCEPT\n");
             return;
         }
         else
         {
             HAS_ERROR = true;
             fprintf (pFile,"Incorrect grammar for Function assign declaration - ERROR");
             printf ("%s\n",STR_NEXT_LINE);
             printf ("ERROR\n");
             fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
             fprintf (pErrorFile,"Message: Incorrect grammar for function assign declaration, check semantics / grammar\n");
             fprintf (pErrorFile,"\n");
             num_errors++;
             return;
         }
     }
     /* Int */
     if (strcmp(Stack[0],"int_optr") == 0)
     {
         if (PARSE_INT_TREE() == true)
         {
             fprintf (pFile,"Integer variable(s) declaration - ACCEPT");
             printf ("ACCEPT\n");
             return;
         }
         else
         {
             HAS_ERROR = true;
             fprintf (pFile,"Incorrect grammar for Integer variable(s) declaration - ERROR");
             printf ("%s\n",STR_NEXT_LINE);
             printf ("ERROR\n");
             fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
             fprintf (pErrorFile,"Message: Incorrect grammar for Integer variable(s) declaration, check semantics / grammar\n");
             fprintf (pErrorFile,"\n");
             num_errors++;
             return;
         }
     }
     
     /* Float */
     if (strcmp(Stack[0],"float_optr") == 0)
     {
         if (PARSE_FLOAT_TREE() == true)
         {
             fprintf (pFile,"Float variable(s) declaration - ACCEPT");
             printf ("ACCEPT\n");
             return;
         }
         else
         {
             HAS_ERROR = true;
             fprintf (pFile,"Incorrect grammar for Float variable(s) declaration - ERROR");
             printf ("%s\n",STR_NEXT_LINE);
             printf ("ERROR\n");
             fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
             fprintf (pErrorFile,"Message: Incorrect grammar for Float variable(s) declaration, check semantics / grammar\n");
             fprintf (pErrorFile,"\n");
             num_errors++;
             return;
         }
     }
     
     /* Boolean */
     if (strcmp(Stack[0],"boolean_optr") == 0)
     {
         if (PARSE_BOOLEAN_TREE() == true)
         {
             fprintf (pFile,"Boolean variable(s) declaration - ACCEPT");
             printf ("ACCEPT\n");
             return;
         }
         else
         {
             HAS_ERROR = true;
             fprintf (pFile,"Incorrect grammar for Boolean variable(s) declaration - ERROR");
             printf ("%s\n",STR_NEXT_LINE);
             printf ("ERROR\n");
             fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
             fprintf (pErrorFile,"Message: Incorrect grammar for Boolean variable(s) declaration, check semantics / grammar\n");
             fprintf (pErrorFile,"\n");
             num_errors++;
             return;
         }
     }
     
     /* Character */
     if (strcmp(Stack[0],"char_optr") == 0)
     {
         if (PARSE_CHAR_TREE() == true)
         {
             fprintf (pFile,"Character variable(s) declaration - ACCEPT");
             printf ("ACCEPT\n");
             return;
         }
         else
         {
             HAS_ERROR = true;
             fprintf (pFile,"Incorrect grammar for Character variable(s) declaration - ERROR");
             printf ("%s\n",STR_NEXT_LINE);
             printf ("ERROR\n");
             fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
             fprintf (pErrorFile,"Message: Incorrect grammar for Character variable(s) declaration, check semantics / grammar\n");
             fprintf (pErrorFile,"\n");
             num_errors++;
             return;
         }
     }
     
     /* String */
     if (strcmp(Stack[0],"string_optr") == 0)
     {
         if (PARSE_STRING_TREE() == true)
         {
             fprintf (pFile,"String variable(s) declaration - ACCEPT");
             printf ("ACCEPT\n");
             return;
         }
         else
         {
             HAS_ERROR = true;
             fprintf (pFile,"Incorrect grammar for String variable(s) declaration - ERROR");
             printf ("%s\n",STR_NEXT_LINE);
             printf ("ERROR\n");
             fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
             fprintf (pErrorFile,"Message: Incorrect grammar for String variable(s) declaration, check semantics / grammar\n");
             fprintf (pErrorFile,"\n");
             num_errors++;
             return;
         }
     }
     
     /* function */
     if (strcmp(Stack[0],"func") == 0)
     {
         if (BRACKET_INDEX > 0)
         {
            HAS_ERROR = true;
            printf ("ERROR: ATTEMPTING TO DECLARE A FUNCTION INSIDE AN UNTERMINATED GROUP. A '}' HAS BEEN LEFT OUT\n");
            printf ("ERROR\n");
            fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
            fprintf (pErrorFile,"Message: Attempting to declare a funcion inside an unterminated group. A '}' has been left out\n");
            fprintf (pErrorFile,"\n");
            num_errors++;
         }   
         if (PARSE_FUNCTION_TREE() == true)
         {
              fprintf (pFile,"Function declaration - ACCEPT");
              printf ("ACCEPT\n");
              return;
         }
         else
         {
               HAS_ERROR = true;
               fprintf (pFile,"Incorrect grammar for function declatarion - ERROR");
               printf ("%s\n",STR_NEXT_LINE);
               printf ("ERROR\n");
               fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
               fprintf (pErrorFile,"Message: Grammar for declaring a function is incorrect\n");
               fprintf (pErrorFile,"\n");
               num_errors++;
               return;
         }
     }
     
     /* Assignments */
     if (STACK_INDEX > 1 && strcmp(Stack[0],"IDENTIFIER") == 0 && strcmp(Stack[1],"ass_val") == 0)
     {
         if (PARSE_ASSIGN_TREE() == true)
         {
              fprintf (pFile,"Assignment declaration - ACCEPT");
              printf ("ACCEPT\n");
              return;
         }
         else
         {
               HAS_ERROR = true;
               fprintf (pFile,"Incorrect grammar for assignment declatarion - ERROR");
               printf ("%s\n",STR_NEXT_LINE);
               printf ("ERROR\n");
               fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
               fprintf (pErrorFile,"Message: Incorrect grammar for an assignment. Please check grammar / semantics\n");
               fprintf (pErrorFile,"\n");
               num_errors++;
               return;
         }
     }
     
     /* Identifiers */
     if (strcmp(Stack[0],"FUNCTION_CALL") == 0)
     {
         if (PARSE_IDENTIFIER_TREE() == true)
         {
              fprintf (pFile,"Identifier declaration - ACCEPT");    
              printf ("ACCEPT\n");
              return;
         }
         else
         {
               HAS_ERROR = true;
               fprintf (pFile,"Incorrect grammar for identifier declatarion - ERROR");
               printf ("%s\n",STR_NEXT_LINE);
               printf ("ERROR\n");
               fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
               fprintf (pErrorFile,"Message: Incorrect gramma for calling functions. Please check semantics / grammar\n");
               fprintf (pErrorFile,"\n");
               num_errors++;
               return;
         }
     }
     
     
     
     /* return */
     
     if (strcmp(Stack[0],"return_optr") == 0)
     {
        if (PARSE_RETURN_TREE() == true)
        {
                fprintf (pFile,"Return declaration - ACCEPT");
                printf ("ACCEPT\n");
                return;
        }
        else
        {
            HAS_ERROR = true;
                fprintf (pFile,"Incorrect grammar for return declaration - ERROR");
                printf ("%s\n",STR_NEXT_LINE);
                printf ("ERROR\n");
                fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
                fprintf (pErrorFile,"Message: Grammar for returning values is incorrect. Please check semantics / grammar\n");
                fprintf (pErrorFile,"\n");
                num_errors++;
                return;
        }
     }
     
     /* if & else */
     
     if (strcmp(Stack[0],"if_optr") == 0)
     {
        if (PARSE_IF_TREE() == true)
        {
                fprintf (pFile,"If declaration - ACCEPT");
                printf ("ACCEPT\n");
                NUM_IFS++;
                return;
        }
        else
        {
                HAS_ERROR = true;
                fprintf (pFile,"Incorrect grammar for if declaration - ERROR");
                printf ("%s\n",STR_NEXT_LINE);
                printf ("ERROR\n");
                fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
                fprintf (pErrorFile,"Message: Grammar for declaring if statements is incorrect. Please check semantics / grammar\n");
                fprintf (pErrorFile,"\n");
                num_errors++;
                return;
        }
     }
     
     if (strcmp(Stack[0],"else_if_optr") == 0)
     {
        if (NUM_IFS == 0)
        {
                HAS_ERROR = true;
                printf ("%s\n",STR_NEXT_LINE);
                printf ("ERROR\n");
                fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
                fprintf (pErrorFile,"Message: else if statement with no preceding if statement\n");
                fprintf (pErrorFile,"\n");
                num_errors++;
                return;
        }
        if (PARSE_IF_TREE() == true)
        {
                fprintf (pFile,"Else If declaration - ACCEPT");
                printf ("ACCEPT\n");
              //  NUM_IFS++;
                return;
        }
        else
        {
                HAS_ERROR = true;
                fprintf (pFile,"Incorrect grammar for else if declaration - ERROR");
                printf ("%s\n",STR_NEXT_LINE);
                printf ("ERROR\n");
                fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
                fprintf (pErrorFile,"Message: Grammar for declaring else if statements is incorrect. Please check semantics / grammar\n");
                fprintf (pErrorFile,"\n");
                num_errors++;
                return;
        }
     }
     
     if (strcmp(Stack[0],"else_optr") == 0)
     {
        if (STACK_INDEX == 1)
        {
              NUM_IFS--;
              if (NUM_IFS < 0)
              {
                 HAS_ERROR = true;
                 fprintf (pFile,"Excess else statement");
                 printf ("%s\n",STR_NEXT_LINE);
                 printf ("ERROR\n");
                 fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
                 fprintf (pErrorFile,"Message: Excess else statement\n");
                 fprintf (pErrorFile,"\n");
                 num_errors++;
                 return;
              }
              else
              {
               fprintf (pFile,"else declaration - ACCEPT");
               printf ("ACCEPT\n");
               return;
              }
        }
     }
     /* while loop */
     
     if (strcmp(Stack[0],"while_optr") == 0)
     {
                                       if (PARSE_WHILE_TREE() == true)
                                       {
                                                              fprintf (pFile,"While declaration - ACCEPT");
                                                              printf ("ACCEPT\n");
                                                              return;
                                       }
                                       else
                                       {
                                           HAS_ERROR = true;
                                           fprintf (pFile,"Incorrect grammar for While declaration - ERROR");
                                           printf ("%s\n",STR_NEXT_LINE);
                                           printf ("ERROR\n");
                                           fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
                                           fprintf (pErrorFile,"Message: Grammar for declaring while statements is incorrect. Please check semantics / grammar\n");
                                           fprintf (pErrorFile,"\n");
                                           num_errors++;
                                           return;
                                       }
     }
     /* for loop */
     
     if (strcmp(Stack[0],"for_optr") == 0)
     {
                                     if (PARSE_FOR_TREE() == true)
                                     {
                                                          fprintf (pFile,"For declaration - ACCEPT");
                                                          printf ("ACCEPT\n");
                                                          return;
                                     }
                                     else
                                     {
                                         HAS_ERROR = true;
                                                          fprintf (pFile,"FOR declaration - ERROR");
                                                          printf ("%s\n",STR_NEXT_LINE);
                                                          printf ("ERROR\n");
                                                          fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
                                                          fprintf (pErrorFile,"Message: Grammar for declaring for statements is incorrect. Please check semantics / grammar\n");
                                                          fprintf (pErrorFile,"\n");
                                                          num_errors++;
                                                          return;
                                     }
     }
     /* do while */
     
	 if (strcmp(Stack[0],"do_while_optr") == 0)
	 {
         if (PARSE_DO_WHILE_TREE() == true)
         {
              fprintf (pFile,"write declaration - ACCEPT");
              printf ("ACCEPT\n");
              return;
         }
         else
         {
				HAS_ERROR = true;
               fprintf (pFile,"Incorrect grammar for do while declatarion - ERROR");
               printf ("%s\n",STR_NEXT_LINE);
               printf ("ERROR\n");
               fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
                fprintf (pErrorFile,"Message: Grammar for declaring do-while statements is incorrect.\n");
                fprintf (pErrorFile,"\n");
                num_errors++;
               return;
         }
	}
     /* read */
     if (strcmp(Stack[0],"read_optr") == 0)
     {
         if (PARSE_READ_TREE() == true)
         {
              fprintf (pFile,"read declaration - ACCEPT");
              printf ("ACCEPT\n");
              return;
         }
         else
         {
               HAS_ERROR = true;
               fprintf (pFile,"Incorrect grammar for read declatarion - ERROR");
               printf ("%s\n",STR_NEXT_LINE);
               printf ("ERROR\n");
               fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
               fprintf (pErrorFile,"Message: Grammar for declaring read statements is incorrect.\n");
               fprintf (pErrorFile,"\n");
               num_errors++;
               return;
         }
     }
     /* write */
     if (strcmp(Stack[0],"write_optr") == 0)
     {
         if (PARSE_WRITE_TREE() == true)
         {
              fprintf (pFile,"write declaration - ACCEPT");
              printf ("ACCEPT\n");
              return;
         }
         else
         {
             HAS_ERROR = true;
               fprintf (pFile,"Incorrect grammar for write declatarion - ERROR");
               printf ("%s\n",STR_NEXT_LINE);
               printf ("ERROR\n");
               fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
                fprintf (pErrorFile,"Message: Grammar for declaring write statements is incorrect.\n");
                fprintf (pErrorFile,"\n");
                num_errors++;
               return;
         }
     }
     
     /* define */
     
     if (strcmp(Stack[0],"number_sign") == 0)
     {
         if (PARSE_DEFINE_TREE() == true)
         {
              fprintf (pFile,"constant declaration - ACCEPT");
              printf ("ACCEPT\n");
              return;
         }
         else
         {
               HAS_ERROR = true;
               fprintf (pFile,"Incorrect grammar for constant declaration");
               printf ("%s\n",STR_NEXT_LINE);
               printf ("ERROR\n");
               fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
                fprintf (pErrorFile,"Message: Grammar for declaring constants is incorrect.\n");
                fprintf (pErrorFile,"\n");
                num_errors++;
               return;
         }
     }
     
	 /* Arrays */
	 
	 if (strcmp(Stack[0],"array_optr") == 0)
	 {
         if (PARSE_ARRAY_TREE() == true)
         {
              fprintf (pFile,"array declaration - ACCEPT");
              printf ("ACCEPT\n");
              return;
         }
         else
         {
               HAS_ERROR = true;
               fprintf (pFile,"Incorrect grammar for array declaration");
               printf ("%s\n",STR_NEXT_LINE);
               printf ("ERROR\n");
               fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
               fprintf (pErrorFile,"Message: Grammar for declaring arrays is incorrect.\n");
               fprintf (pErrorFile,"\n");
               num_errors++;
               return;
         }
	 }
	 
     /* Unknown...*/
     
     if (strcmp(Stack[0],"set_array") == 0)
     {
        if (PARSE_SET_ARRAY_TREE())
        {
              fprintf (pFile,"Set array value declaration - ACCEPT");
              printf ("ACCEPT\n");
              return;
        }
        else
        {
               HAS_ERROR = true;
               fprintf (pFile,"Incorrect grammar for setting array declaration");
               printf ("%s\n",STR_NEXT_LINE);
               printf ("ERROR\n");
               fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
               fprintf (pErrorFile,"Message: Grammar for declaring array values is incorrect.\n");
               fprintf (pErrorFile,"\n");
               num_errors++;
               return;
        }
     }
     
     if (strcmp(Stack[0],"get_array") == 0)
     {
        if (PARSE_GET_ARRAY_TREE())
        {
              fprintf (pFile,"Get array value declaration - ACCEPT");
              printf ("ACCEPT\n");
              return;
        }
        else
        {
               HAS_ERROR = true;
               fprintf (pFile,"Incorrect grammar for getting array declaration");
               printf ("%s\n",STR_NEXT_LINE);
               printf ("ERROR\n");
               fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
               fprintf (pErrorFile,"Message: Grammar for getting array values is incorrect.\n");
               fprintf (pErrorFile,"\n");
               num_errors++;
               return;
        }
     }
     
     if (strcmp(Stack[0],"UNDEFINED") == 0)
     {
            HAS_ERROR = true;
            fprintf (pFile,"Unknown declaration");                        
            printf ("%s\n",STR_NEXT_LINE);
            printf ("ERROR\n");
            fprintf (pErrorFile,"Line %d: %s\n",Line_Number,STR_NEXT_LINE);
            fprintf (pErrorFile,"Message: Unknown declaration\n");
            fprintf (pErrorFile,"\n");
            num_errors++;
            return;        
     }
	 
     else
     return;
}
/**** Inspired to do a bottom up parser c/o Sir Danniel "Paxi" Alcantara ****/

/******************************** INTERPRETER SECTION *****************************************/

     
void INTERPRET_WRITE(String Source);
void INTERPRET_READ(String Source);
void INTERPRET_INT_IDENTIFY(String Source);
void INTERPRET_IF_STATEMENT(String Source,int *counter);
void INTERPRET_IF_STATEMENT_FUNCTION(String Source,int *counter,int index_func,int VARIABLES_START,bool *hasReturned);
void INTERPRET_WHILE_STATEMENT(String Source,int *counter);
void INTERPRET_WHILE_STATEMENT_FUNCTION(String Source,int *counter,int index_func,int VARIABLES_START,bool *hasReturned);
void INTERPRET_FOR_STATEMENT(String Source,int *counter);
void INTERPRET_FOR_STATEMENT_FUNCTION(String Source,int *counter,int index_func,int VARIABLES_START,bool *hasReturned);

void INTERPRET_ARRAY_ASSIGNMENT(String cand);
void INTERPRET_ARRAY_GET(String cand);

void CALL_FUNCTION(String CAND);
void INTERPRET_DO_WHILE_STATEMENT(String Source,int *counter);
void INTERPRET_DO_WHILE_STATEMENT_FUNCTION(String Source,int *counter,int index_func,int VARIABLES_START,bool *hasReturned);
void INTERPRET_ASSIGN_FUNCTION_STATEMENT (String CAND);
void INTERPRET_RETURN_STATEMENT(String Source,int index_func,int VARIABLES_START);

bool isHigher(char c1,char c2)
{
     if (c1 == '+' && (c2 == '*' || c2 == '/' || c2 == '$'))
        return false;
     else if (c1 == '-' && (c2 == '*' || c2 == '/' || c2 == '$'))
        return false;
     else
        return true;
}

void NUMBER_PUSH(int num)
{
     number_stack[number_stack_index] = num;
     number_stack_index++;
}

void PRINT_NUMBER_STACK()
{
     int i;
     printf ("Stack contents: \n");
     for (i = number_stack_index-1;i >= 0;i--)
         printf ("%d\n",number_stack[i]);
}

int NUMBER_POP()
{
      int fNum = number_stack[number_stack_index-1];
      number_stack_index--;
      return fNum;
}

void OPERATOR_PUSH(char ch)
{
     operator_stack[operator_stack_index] = ch;
     operator_stack_index++;
}

char OPERATOR_PEEK()
{
     if (operator_stack_index <= 0)
        return ' ';
     return operator_stack[operator_stack_index-1];
}

char OPERATOR_POP()
{
     if (operator_stack_index <= 0)
        return ' ';
     char ch = operator_stack[operator_stack_index-1];
     operator_stack_index--;
     
     return ch;
}

void OPERATOR_REFRESH()
{
     operator_stack_index = 0;
     number_stack_index = 0;
}

void INFIX_TO_POSTFIX(String Source,String final_result)
{
     String empty;
     String dummy;
     String value;
     int value_index;
     
     int length = strlen(Source);
     
     strcpy(empty,"");
     strcpy(dummy,"");
     strcpy(operator_stack,"");
     
     int empty_index = 0;
     int dummy_index = 0;
     int i = 0;
     
   //  printf ("INFIX TO POSTFIX: %s\n",Source);
     
     OPERATOR_REFRESH();
     
     for (int i = 0;i < length;i++)
     {
         if (Source[i] == '-' && i == 0)
            Source[i] = '~';
         else if (i > 0 && Source[i] == '-' && (Source[i-1] == '+' || Source[i-1] == '/' || Source[i-1] == '-' || Source[i-1] == '*' || Source[i-1] == '\%'))
              Source[i] = '~';
     }
     
     while (i < length)
     {
           if (Source[i] >= '0' && Source[i] <= '9' || Source[i] == '~')
           {
                         strcpy(dummy,"");
                         dummy_index = 0;
                         while (i < length && ((Source[i] >= '0' && Source[i] <= '9') || Source[i] == '~'))
                         {
                               dummy[dummy_index] = Source[i];
                               i++;
                               dummy_index++;
                         }
                         dummy[dummy_index] = ' ';
                         dummy_index++;
                         dummy[dummy_index] = '\0';
                         strcat(empty,dummy);
                         empty_index = strlen(empty);
                    //     printf ("%s\n",empty);
           }
           else if (IS_ALPHA(Source[i]) || IS_UNDERSCORE(Source[i]))
           {
                         strcpy(dummy,"");
                         dummy_index = 0;
                         while (i < length && (IS_ALPHA(Source[i]) || IS_UNDERSCORE(Source[i]) || IS_DIGIT(Source[i])))
                         {
                               dummy[dummy_index] = Source[i];
                               i++;
                               dummy_index++;
                         }
                         dummy[dummy_index] = '\0';
                       //  printf ("Getting the variable for %s\n",dummy);
                         GET_VARIABLE_VALUE(dummy,value);
                      //   printf ("Obtained!\n");
                         value_index = strlen(value);
                         value[value_index] = ' ';
                         value_index++;
                         value[value_index] = '\0';
                         strcat(empty,value);
                         empty_index = strlen(empty);
           }
           else
           {
               if (Source[i] == '(')
               {
                         OPERATOR_PUSH(Source[i]);
               }
               else if (Source[i] == ')')
               {
                    char cand = OPERATOR_POP();
                    while (cand != '(')
                    {
                          empty[empty_index] = cand;
                          empty_index++;
                          empty[empty_index] = ' ';
                          empty_index++;
                          empty[empty_index] = '\0';
                          cand = OPERATOR_POP();
                    }
               }
               else if (Source[i] == '+' || Source[i] == '-' || Source[i] == '*' || Source[i] == '/' || Source[i] == '$')
               {
                    if (isHigher(Source[i],OPERATOR_PEEK()))
                       OPERATOR_PUSH(Source[i]);
                    else
                    {
                        char ch = OPERATOR_POP();
                    //    printf ("%c\n",ch);
                        empty[empty_index] = ch;
                        empty_index++;
                        empty[empty_index] = ' ';
                        empty_index++;
                        empty[empty_index] = '\0';
                        OPERATOR_PUSH(Source[i]);
                     //   printf ("%s\n",empty);
                    }
               }
               i++;
           }
   //        printf("%s\n",empty);
     }
     while(operator_stack_index > 0)
     {
                                empty[empty_index] = OPERATOR_POP();
                                empty_index++;
                                empty[empty_index] = ' ';
                                empty_index++;
     }
     
     empty[empty_index] = '\0';
  //   printf("%s\n",empty);
     strcpy(final_result,empty);
}

void EVALUATE_EQUATION(String Source,String output)
{
     int i = 0;
     int final;
     
     String empty;
     strcpy(empty,Source);
     INFIX_TO_POSTFIX(empty,Source);
     int length = strlen(Source);
     String dummy;
     int dummy_index;
     
     for (int j = 0;j < length;j++)
     {
         if (Source[j] == '-' && IS_DIGIT(Source[j+1]))
            Source[j] = '~';
     }
    // printf ("%s\n",Source);
     while (i < length)
     {
           if (Source[i] >= '0' && Source[i] <= '9' || Source[i] == '~')
           {
                         dummy_index = 0;
                         while (Source[i] != ' ')
                         {
                               dummy[dummy_index] = Source[i];
                               i++;
                               dummy_index++;
                         }
                         dummy[dummy_index] = '\0';
                         if (dummy[0] == '~')
                            dummy[0] = '-';
                      //   printf("Number obtained: %s\n",dummy);
                         NUMBER_PUSH(atoi(dummy));
           }
           else if (Source[i] == '+' || Source[i] == '-' || Source[i] == '/' || Source[i] == '*' || Source[i] == '$')
           {
                int fNum1,fNum2;
                
                switch(Source[i])
                {
                        case '+':
                             fNum2 = NUMBER_POP();
                             fNum1 = NUMBER_POP();
                             NUMBER_PUSH(fNum1+fNum2);
                         //    printf ("Result: %d + %d = %d\n",fNum1,fNum2,fNum1+fNum2);
                             break;   
                             
                        case '-':
                             fNum2 = NUMBER_POP();
                             fNum1 = NUMBER_POP();
                             NUMBER_PUSH(fNum1-fNum2);
                         //    printf ("Result: %d - %d = %d\n",fNum1,fNum2,fNum1-fNum2);
                             break;         
                        
                        case '*':
                             fNum2 = NUMBER_POP();
                             fNum1 = NUMBER_POP();
                             NUMBER_PUSH(fNum1*fNum2);
                        //     printf ("Result: %d * %d = %d\n",fNum1,fNum2,fNum1*fNum2);
                             break;   
                        
                        case '/':
                             fNum2 = NUMBER_POP();
                             fNum1 = NUMBER_POP();
                             if (fNum2 == 0)
                             {
                                       RUNTIME_ERROR = true;
                                       EXCEPTION_TYPE = DIVISION_BY_ZERO_EXCEPTION;
                                       if (WATCH_AND_TRACE)
                                       {
                                           printf ("<RUNTIME_ERROR: DIVISION_BY_ZERO_EXCEPTION>\n");
                                           getch();
                                       }
                                       return;
                             }
                             NUMBER_PUSH(fNum1/fNum2);
                            // printf ("Result: %d / %d = %d\n",fNum1,fNum2,fNum1/fNum2);
                             break;   
                        
                        case '$':
                             fNum2 = NUMBER_POP();
                             fNum1 = NUMBER_POP();
                             NUMBER_PUSH(fNum1%fNum2);
                             break;   
                }
                i++;
           }
           else if (Source[i] == ' ')
                i++;
     //      PRINT_NUMBER_STACK();
     }
     final = NUMBER_POP();
     String finish;
     itoa(final,finish,10);
  //   printf ("finish initial: %s\n",finish);
     for (int i = 0;i < strlen(finish);i++)
         if (finish[i] == '~')
            finish[i] = '-';
 //    printf("finish cleaned: %s\n",finish);
     strcpy(output,finish);
 //    printf("Given value: %s\n",output);
}

/************************************
FLOAT ISSUES
************************************/

void NUMBER_PUSH_FLOAT(float num)
{
     number_stack_float[number_stack_float_index] = num;
     number_stack_float_index++;
}

void PRINT_NUMBER_STACK_FLOAT()
{
     int i;
     printf ("Stack contents: \n");
     for (i = number_stack_float_index-1;i >= 0;i--)
         printf ("%d\n",number_stack_float[i]);
}

float NUMBER_POP_FLOAT()
{
      float fNum = number_stack_float[number_stack_float_index-1];
      number_stack_float_index--;
      return fNum;
}

void OPERATOR_PUSH_FLOAT(char ch)
{
     operator_stack_float[operator_stack_float_index] = ch;
     operator_stack_float_index++;
}

char OPERATOR_PEEK_FLOAT()
{
     if (operator_stack_float_index <= 0)
        return ' ';
     return operator_stack_float[operator_stack_float_index-1];
}

char OPERATOR_POP_FLOAT()
{
     if (operator_stack_float_index <= 0)
        return ' ';
     char ch = operator_stack_float[operator_stack_float_index-1];
     operator_stack_float_index--;
     
     return ch;
}

void OPERATOR_REFRESH_FLOAT()
{
     operator_stack_float_index = 0;
     number_stack_float_index = 0;
}

void INFIX_TO_POSTFIX_FLOAT(String Source,String final_result)
{
     String empty;
     String dummy;
     String value;
     int value_index;
     
     int length = strlen(Source);
     
     strcpy(empty,"");
     strcpy(dummy,"");
     strcpy(operator_stack,"");
     
     int empty_index = 0;
     int dummy_index = 0;
     int i = 0;
     
    // printf ("INFIX TO POSTFIX: %s\n",Source);
     
     OPERATOR_REFRESH_FLOAT();
     
     for (int i = 0;i < length;i++)
     {
         if (Source[i] == '-' && i == 0)
            Source[i] = '~';
         else if (i > 0 && Source[i] == '-' && (Source[i-1] == '+' || Source[i-1] == '/' || Source[i-1] == '-' || Source[i-1] == '*' || Source[i-1] == '\%'))
              Source[i] = '~';
     }
     
     while (i < length)
     {
           if (Source[i] >= '0' && Source[i] <= '9' || Source[i] == '~' || Source[i] == '.')
           {
                         strcpy(dummy,"");
                         dummy_index = 0;
                         while (i < length && ((Source[i] >= '0' && Source[i] <= '9') || Source[i] == '~' || Source[i] == '.'))
                         {
                               dummy[dummy_index] = Source[i];
                               i++;
                               dummy_index++;
                         }
                         dummy[dummy_index] = ' ';
                         dummy_index++;
                         dummy[dummy_index] = '\0';
                         strcat(empty,dummy);
                         empty_index = strlen(empty);
                //         printf ("%s\n",empty);
           }
           else if (IS_ALPHA(Source[i]) || IS_UNDERSCORE(Source[i]))
           {
                         strcpy(dummy,"");
                         dummy_index = 0;
                         while (i < length && (IS_ALPHA(Source[i]) || IS_UNDERSCORE(Source[i]) || IS_DIGIT(Source[i])))
                         {
                               dummy[dummy_index] = Source[i];
                               i++;
                               dummy_index++;
                         }
                         dummy[dummy_index] = '\0';
                       //  printf ("Getting the variable for %s\n",dummy);
                         GET_VARIABLE_VALUE(dummy,value);
                      //   printf ("Obtained!\n");
                         value_index = strlen(value);
                         value[value_index] = ' ';
                         value_index++;
                         value[value_index] = '\0';
                         strcat(empty,value);
                         empty_index = strlen(empty);
           }
           else
           {
               if (Source[i] == '(')
               {
                         OPERATOR_PUSH_FLOAT(Source[i]);
               }
               else if (Source[i] == ')')
               {
                    char cand = OPERATOR_POP_FLOAT();
                    while (cand != '(')
                    {
                          empty[empty_index] = cand;
                          empty_index++;
                          empty[empty_index] = ' ';
                          empty_index++;
                          empty[empty_index] = '\0';
                          cand = OPERATOR_POP_FLOAT();
                    }
               }
               else if (Source[i] == '+' || Source[i] == '-' || Source[i] == '*' || Source[i] == '/' || Source[i] == '$')
               {
                    if (isHigher(Source[i],OPERATOR_PEEK_FLOAT()))
                       OPERATOR_PUSH_FLOAT(Source[i]);
                    else
                    {
                        char ch = OPERATOR_POP_FLOAT();
                    //    printf ("%c\n",ch);
                        empty[empty_index] = ch;
                        empty_index++;
                        empty[empty_index] = ' ';
                        empty_index++;
                        empty[empty_index] = '\0';
                        OPERATOR_PUSH_FLOAT(Source[i]);
                     //   printf ("%s\n",empty);
                    }
               }
               i++;
           }
   //        printf("%s\n",empty);
     }
     while(operator_stack_float_index > 0)
     {
                                empty[empty_index] = OPERATOR_POP_FLOAT();
                                empty_index++;
                                empty[empty_index] = ' ';
                                empty_index++;
     }
     
     empty[empty_index] = '\0';
 //    printf("%s\n",empty);
     strcpy(final_result,empty);
}

void EVALUATE_EQUATION_FLOAT(String Source,String output)
{
     int i = 0;
     float final;
     
     String empty;
     strcpy(empty,Source);
     INFIX_TO_POSTFIX_FLOAT(empty,Source);
     int length = strlen(Source);
     String dummy;
     int dummy_index;
     
     for (int j = 0;j < length;j++)
     {
         if (Source[j] == '-' && IS_DIGIT(Source[j+1]))
            Source[j] = '~';
     }
  //   printf ("%s\n",Source);
     while (i < length)
     {
           if (Source[i] >= '0' && Source[i] <= '9' || Source[i] == '~' || Source[i] == '.')
           {
                         dummy_index = 0;
                         while (Source[i] != ' ')
                         {
                               dummy[dummy_index] = Source[i];
                               i++;
                               dummy_index++;
                         }
                         dummy[dummy_index] = '\0';
                         if (dummy[0] == '~')
                            dummy[0] = '-';
                      //   printf("Number obtained: %s\n",dummy);
                         NUMBER_PUSH_FLOAT((float)atof(dummy));
           }
           else if (Source[i] == '+' || Source[i] == '-' || Source[i] == '/' || Source[i] == '*' || Source[i] == '$')
           {
                float fNum1,fNum2;
                
                switch(Source[i])
                {
                        case '+':
                             fNum2 = NUMBER_POP_FLOAT();
                             fNum1 = NUMBER_POP_FLOAT();
                             NUMBER_PUSH_FLOAT(fNum1+fNum2);
                         //    printf ("Result: %d + %d = %d\n",fNum1,fNum2,fNum1+fNum2);
                             break;   
                             
                        case '-':
                             fNum2 = NUMBER_POP_FLOAT();
                             fNum1 = NUMBER_POP_FLOAT();
                             NUMBER_PUSH_FLOAT(fNum1-fNum2);
                         //    printf ("Result: %d - %d = %d\n",fNum1,fNum2,fNum1-fNum2);
                             break;         
                        
                        case '*':
                             fNum2 = NUMBER_POP_FLOAT();
                             fNum1 = NUMBER_POP_FLOAT();
                             NUMBER_PUSH_FLOAT(fNum1*fNum2);
                        //     printf ("Result: %d * %d = %d\n",fNum1,fNum2,fNum1*fNum2);
                             break;   
                        
                        case '/':
                             fNum2 = NUMBER_POP_FLOAT();
                             fNum1 = NUMBER_POP_FLOAT();
                             if (fNum2 == 0)
                             {
                                       RUNTIME_ERROR = true;
                                       EXCEPTION_TYPE = DIVISION_BY_ZERO_EXCEPTION;
                                       if (WATCH_AND_TRACE)
                                       {
                                           printf ("<RUNTIME_ERROR: DIVISION_BY_ZERO_EXCEPTION>\n");
                                           getch();
                                       }
                                       return;
                             }
                             NUMBER_PUSH_FLOAT(fNum1/fNum2);
                            // printf ("Result: %d / %d = %d\n",fNum1,fNum2,fNum1/fNum2);
                             break;   
                        
                       /* case '$':
                             fNum2 = NUMBER_POP();
                             fNum1 = NUMBER_POP();
                             NUMBER_PUSH(fNum1%fNum2);
                             break;   */
                }
                i++;
           }
           else if (Source[i] == ' ')
                i++;
     //      PRINT_NUMBER_STACK();
     }
     final = NUMBER_POP_FLOAT();
     String finish;
     sprintf(finish,"%f",final);
     for (int i = 0;i < strlen(finish);i++)
         if (finish[i] == '~')
            finish[i] = '-';
     strcpy(output,finish);
}

void INTERPRET_WRITE(String Source)
{
     String empty;
     String value;
     bool end = false;
     bool end2 = false;
     
     strcpy(empty,"");
     strcpy(value,"");
     
     if (RUNTIME_ERROR)
        return;
     if (WATCH_AND_TRACE)
     {
        printf ("<Executing: %s>\n",Source);
        getch();
     }
     int i = 0;
     int empty_index = 0;
     while (Source[i] != '(')
           i++;
     i++;
     
     while (!end)
     {
         if (Source[i] == '\"')
         {
                       i++;
                       while (end2 == false)
                       {
                               if (Source[i] == '\"' && Source[i-1] != '\\')
                               {
                                             end2 = true;
                                             break;
                               }
                               else if (Source[i] == '\"' && Source[i-1] == '\\')
                               {
                                    empty[empty_index-1] = '\"';
                                    i++;
                               }
                               else if (i > 0 && Source[i] == 'n' && Source[i-1] == '\\')
                               {
                                    empty[empty_index-1] = '\n';
                                    i++;
                               }
                               else
                               {
                                   empty[empty_index] = Source[i];
                                   empty_index++;
                                   i++;
                               }
                       }
                       empty[empty_index] = '\0';
                       i++;

                       printf("%s",empty);
                       empty_index = 0;
                       if (Source[i] == ')')
                          end = true;
         }
         else if (Source[i] == '+' || Source[i] == ' ')
         {
              i++;
              end2 = false;
         }
         else
         {
             while (Source[i] != ')' && Source[i] != '+' && Source[i] != ' ')
             {
                   empty[empty_index]  = Source[i];
                   empty_index++;
                   i++;
             }
             empty[empty_index] = '\0';
             if (Source[i] == ')')
                end = true;
             GET_VARIABLE_VALUE(empty,value);
             printf("%s",value);
             empty_index = 0;
             end2 = false;
         }
     }
    /* if (strcmp(value,"") == 0)
     {
         empty[empty_index] = '\0';
         int end_size = strlen(empty);
         for (int i = 0;i < end_size;i++)
         {
             if (empty[i] == '\\')
             {
                          i++;
                          if (empty[i] == 'n')
                                       printf("\n");
                          else
                                       printf("%c",empty[i]);
             }
             else
                 printf ("%c",empty[i]);
         }
     }
     else
         printf("%s",value);*/
}

bool VALID_INTEGER_NUMBER(String cand)
{
     int i,v = strlen(cand);
     for (i = 0;i < v;i++)
     {
         if (cand[i] >= '0' && cand[i] <= '9')
            continue;
         else if (i == 0 && cand[i] == '-')
            continue;
         else
            return false;
     }
     return true;
}

void INTERPRET_READ(String Source)
{
     String empty;
     String input;
     int empty_index;
     int i = 0;
     
     if (RUNTIME_ERROR)
        return;
     if (WATCH_AND_TRACE)
     {
        printf ("<Executing: %s>\n",Source);
        getch();
     }
     empty_index = 0;
     
     while (Source[i] != '(')
           i++;
     i++;
     while (Source[i] != ')')
     {
           empty[empty_index] = Source[i];
           i++;
           empty_index++;
     }
     empty[empty_index] = '\0';
     gets(input);
     if (GET_VARIABLE_TYPE(empty) == 0)
     {
        if (!VALID_INTEGER_NUMBER(input))
        {
          
           RUNTIME_ERROR = true;
           EXCEPTION_TYPE = NUMBER_FORMAT_EXCEPTION;
           if (WATCH_AND_TRACE)
           {
                               printf ("<RUNTIME_ERROR: NUMBER_FORMAT_EXCEPTION>\n");
                               getch();
           }
           return;
        }
     }
     UPDATE_VARIABLE(empty,input);
}

void INTERPRET_FLOAT_IDENTIFY(String Source)
{
     String empty;
	 String value;
	 int Source_length = strlen(Source);
     bool end = false;
     bool end2 = false;
     
     strcpy(empty,"");
	 strcpy(value,"");
	 
	 if (RUNTIME_ERROR)
	    return;
     int i = 0;
	 int empty_index = 0;
	 int value_index = 0;
     while (Source[i] != ' ')
           i++;
     i++;
     
     while (!end)
     {
		end2 = false;
		empty_index = 0;
		value_index = 0;
		   while (Source[i] != ',' && Source[i] != ';')
		   {
				while (!end2)
				{
					if (Source[i] == ' ' || Source[i] == '=' || Source[i] == ',' || Source[i] == ';')
					{
						end2 = true;
						break;
					}
					empty[empty_index] = Source[i];
					i++;
					empty_index++;
				}
				empty[empty_index] = '\0';
				if (Source[i] == ',' || Source[i] == ';')
				{
					NEW_VARIABLE(FLOAT,empty,"null");
				}
				else
				{
					if (Source[i] == ' ')
						i++;
					if (Source[i] == '=')
					{
						i++;
						while (Source[i] == ' ')
							i++;
						while (Source[i] != ',' && Source[i] != ';')
						{
							value[value_index] = Source[i];
							i++;
							value_index++;
						}
						value[value_index] = '\0';
						NEW_VARIABLE(FLOAT,empty,value);
					}
				}
		   }
		   if (i >= Source_length | Source[i] == ';')
		   {
				end = true; break;
		   }
		   i++;
     }
}

void INTERPRET_STRING_IDENTIFY(String Source)
{
     String empty;
	 String value;
	 int Source_length = strlen(Source);
     bool end = false;
     bool end2 = false;
     
     if (RUNTIME_ERROR)
        return;
        
     strcpy(empty,"");
	 strcpy(value,"");
     int i = 0;
	 int empty_index = 0;
	 int value_index = 0;
     while (Source[i] != ' ')
           i++;
     i++;
     
     while (!end)
     {
		end2 = false;
		empty_index = 0;
		value_index = 0;
		   while (Source[i] != ',' && Source[i] != ';')
		   {
				while (!end2)
				{
					if (Source[i] == ' ' || Source[i] == '=' || Source[i] == ',' || Source[i] == ';')
					{
						end2 = true;
						break;
					}
					empty[empty_index] = Source[i];
					i++;
					empty_index++;
				}
				empty[empty_index] = '\0';
				if (Source[i] == ',' || Source[i] == ';')
				{
					NEW_VARIABLE(STRING,empty,"null");
				}
				else
				{
					if (Source[i] == ' ')
						i++;
					if (Source[i] == '=')
					{
						i++;
						while (Source[i] == ' ')
							i++;
						while (Source[i] != ',' && Source[i] != ';')
						{
							value[value_index] = Source[i];
							i++;
							value_index++;
						}
						value[value_index] = '\0';
						if (value[0] <= '0' || value[0] >= '9')
						{
                           String bald;
                           strcpy(bald,value);
						   GET_VARIABLE_VALUE(bald,value);
                        }
						NEW_VARIABLE(STRING,empty,value);
					}
				}
		   }
		   if (i >= Source_length | Source[i] == ';')
		   {
				end = true; break;
		   }
		   i++;
     }
}

void INTERPRET_INT_IDENTIFY(String Source)
{
     String empty;
	 String value;
	 int Source_length = strlen(Source);
     bool end = false;
     bool end2 = false;
     
     
     strcpy(empty,"");
	 strcpy(value,"");
     int i = 0;
	 int empty_index = 0;
	 int value_index = 0;
     while (Source[i] != ' ')
           i++;
     i++;
     
     while (!end)
     {
		end2 = false;
		empty_index = 0;
		value_index = 0;
		   while (Source[i] != ',' && Source[i] != ';')
		   {
				while (!end2)
				{
					if (Source[i] == ' ' || Source[i] == '=' || Source[i] == ',' || Source[i] == ';')
					{
						end2 = true;
						break;
					}
					empty[empty_index] = Source[i];
					i++;
					empty_index++;
				}
				empty[empty_index] = '\0';
				if (Source[i] == ',' || Source[i] == ';')
				{
					NEW_VARIABLE(INT,empty,"null");
				}
				else
				{
					if (Source[i] == ' ')
						i++;
					if (Source[i] == '=')
					{
						i++;
						while (Source[i] == ' ')
							i++;
						while (Source[i] != ',' && Source[i] != ';')
						{
							value[value_index] = Source[i];
							i++;
							value_index++;
						}
						value[value_index] = '\0';
						if (value[0] <= '0' || value[0] >= '9')
						{
                           String bald;
                           strcpy(bald,value);
						   GET_VARIABLE_VALUE(bald,value);
                        }
						NEW_VARIABLE(INT,empty,value);
					}
				}
		   }
		   if (i >= Source_length | Source[i] == ';')
		   {
				end = true; break;
		   }
		   i++;
     }
}

void INTERPRET_INT_IDENTIFY_FUNCTION(String Source,int func_index)
{
     String empty;
	 String value;
	 int Source_length = strlen(Source);
     bool end = false;
     bool end2 = false;
     
     
     strcpy(empty,"");
	 strcpy(value,"");
     int i = 0;
	 int empty_index = 0;
	 int value_index = 0;
     while (Source[i] != ' ')
           i++;
     i++;
     
     while (!end)
     {
		end2 = false;
		empty_index = 0;
		value_index = 0;
		   while (Source[i] != ',' && Source[i] != ';')
		   {
				while (!end2)
				{
					if (Source[i] == ' ' || Source[i] == '=' || Source[i] == ',' || Source[i] == ';')
					{
						end2 = true;
						break;
					}
					empty[empty_index] = Source[i];
					i++;
					empty_index++;
				}
				empty[empty_index] = '\0';
				if (Source[i] == ',' || Source[i] == ';')
				{
					NEW_VARIABLE(INT,empty,"null");
				}
				else
				{
					if (Source[i] == ' ')
						i++;
					if (Source[i] == '=')
					{
						i++;
						while (Source[i] == ' ')
							i++;
						while (Source[i] != ',' && Source[i] != ';')
						{
							value[value_index] = Source[i];
							i++;
							value_index++;
						}
						value[value_index] = '\0';
						if (value[0] <= '0' || value[0] >= '9')
						{
                           String bald;
                           strcpy(bald,value);
						   GET_VARIABLE_VALUE(bald,value);
                        }
						NEW_VARIABLE(INT,empty,value);
					}
				}
		   }
		   if (i >= Source_length | Source[i] == ';')
		   {
				end = true; break;
		   }
		   i++;
     }
}

void INTERPRET_CHAR_IDENTIFY(String Source)
{
     String empty;
	 String value;
	 int Source_length = strlen(Source);
     bool end = false;
     bool end2 = false;
     
     
     strcpy(empty,"");
	 strcpy(value,"");
     int i = 0;
	 int empty_index = 0;
	 int value_index = 0;
     while (Source[i] != ' ')
           i++;
     i++;
     
     while (!end)
     {
		end2 = false;
		empty_index = 0;
		value_index = 0;
		   while (Source[i] != ',' && Source[i] != ';')
		   {
				while (!end2)
				{
					if (Source[i] == ' ' || Source[i] == '=' || Source[i] == ',' || Source[i] == ';')
					{
						end2 = true;
						break;
					}
					empty[empty_index] = Source[i];
					i++;
					empty_index++;
				}
				empty[empty_index] = '\0';
				if (Source[i] == ',' || Source[i] == ';')
				{
					NEW_VARIABLE(CHAR,empty,"null");
				}
				else
				{
					if (Source[i] == ' ')
						i++;
					if (Source[i] == '=')
					{
						i++;
						while (Source[i] == ' ')
							i++;
						if (Source[i] == '\'')
						   i++;
						while (Source[i] != ',' && Source[i] != ';' && Source[i] != '\'')
						{
							value[value_index] = Source[i];
							i++;
							value_index++;
						}
						if (Source[i] == '\'')
						   i++;
						value[value_index] = '\0';
                        String bald;
                        strcpy(bald,value);
                  //      GET_VARIABLE_VALUE(bald,value);
						NEW_VARIABLE(CHAR,empty,value);
					}
				}
		   }
		   if (i >= Source_length || Source[i] == ';')
		   {
				end = true; break;
		   }
		   i++;
     }
}

void INTERPRET_ASSIGN_STATEMENT(String Source)
{
     int Source_Index = 0;
     int Source_Size = strlen(Source);
     int Target_Index = 0;
     
     String variable_name;
     String new_value;
     String temp_new_value;
     
     strcpy(variable_name,"");
     strcpy(new_value,"");
     
     int i = 0;
     
     int variable_index = 0;
     while (Source[i] != ' ' && Source[i] != '=')
     {
           variable_name[variable_index] = Source[i];
           i++;
           variable_index++;
     }
     variable_name[variable_index] = '\0';
     
     variable_index = 0;
     if (Source[i] == ' ')
        i++;
     if (Source[i] == '=')
        i++;
     if (Source[i] == ' ')
        i++;
     while (i < Source_Size && Source[i] != ';')
     {
           new_value[variable_index] = Source[i];
           i++;
           variable_index++;
     }
     new_value[variable_index] = '\0';
     if (GET_VARIABLE_TYPE(variable_name) == 0)
     {
      //  printf ("Not!\n");
        strcpy(temp_new_value,new_value);
        EVALUATE_EQUATION(temp_new_value,new_value);
        UPDATE_VARIABLE(variable_name,new_value);
     }
     else if (GET_VARIABLE_TYPE(variable_name) == FLOAT)
     {
      //  printf ("Not!\n");
        strcpy(temp_new_value,new_value);
        EVALUATE_EQUATION_FLOAT(temp_new_value,new_value);
        UPDATE_VARIABLE(variable_name,new_value);
     }
     else
         UPDATE_VARIABLE(variable_name,new_value);
}

void INTERPRET_ASSIGN_STATEMENT_FOR(String Source)
{
     int Source_Index = 0;
     int Source_Size = strlen(Source);
     int Target_Index = 0;
     
     String variable_name;
     String new_value;
     String temp_new_value;
     
     strcpy(variable_name,"");
     strcpy(new_value,"");
     
     int i = 0;
     
     int variable_index = 0;
     while (Source[i] != ' ' && Source[i] != '=')
     {
           variable_name[variable_index] = Source[i];
           i++;
           variable_index++;
     }
     variable_name[variable_index] = '\0';
     
     variable_index = 0;
     if (Source[i] == ' ')
        i++;
     if (Source[i] == '=')
        i++;
     if (Source[i] == ' ')
        i++;
     while (i < Source_Size && Source[i] != ')')
     {
           new_value[variable_index] = Source[i];
           i++;
           variable_index++;
     }
     new_value[variable_index] = '\0';
     if (GET_VARIABLE_TYPE(variable_name) == 0)
     {
        strcpy(temp_new_value,new_value);
        EVALUATE_EQUATION(temp_new_value,new_value);
        UPDATE_VARIABLE(variable_name,new_value);
     }
     else
         UPDATE_VARIABLE(variable_name,new_value);
}

void GET_FIRST_TOKEN(String Source,String Target)
{
     int Source_Index = 0;
     int Source_Size = strlen(Source);
     int Target_Index = 0;
     strcpy(Target,"");
     
     while (Source_Index < Source_Size && Source[Source_Index] != ' ')
     {
           Target[Target_Index] = Source[Source_Index];
           Target_Index++;
           Source_Index++;
     }
     Target[Target_Index] = '\0';
}

void INTERPRET_IF_STATEMENT(String Source,int *counter)
{
     String empty;
     String value;
     String cond1;
     String op;
     String cond2;
     String interpreter_candidate;
     String s1,s2;
     
     bool end = false;
     bool end2 = false;
     bool cond_verdict = true;
     
     bool bor = false;
     bool band = false;
     
     int nSum = 0,nEnd;
     int start = *counter;
     int finVerdict = 0;
     int conVerdict = 0;
     strcpy(empty,"");
     strcpy(value,"");
     int i = 0;
     int empty_index = 0;
     while (Source[i] != '(')
           i++;
     i++;
     
     while (!end)
     {
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != '>' && Source[i] != '<' && Source[i] != '=')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond1);
           empty_index = 0;
           if (Source[i] == '>' || Source[i] == '<' || Source[i] == '=')
           {
                empty[empty_index] = Source[i];
                i++;
                empty_index++;
                if (Source[i] == '=')
                {
                              empty[empty_index] = Source[i];
                              i++;
                              empty_index++;
                }   
                empty[empty_index] = '\0';
                strcpy(op,empty);
                if (Source[i] == ' ')
                   i++;      
           }
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != ')' && Source[i] != '|' && Source[i] != '&')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond2);
           if (Source[i] == ')')
              end = true;
           
           if (strcmp(op,">") == 0)
           {
              if (atoi(cond1) - atoi(cond2) > 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;;
           }
           else if (strcmp (op,"<") == 0)
           {
              if (atoi(cond1) - atoi(cond2) < 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,">=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) >= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"<=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) <= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"==") == 0)
           {
              if (atoi(cond1) - atoi(cond2) == 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           
           if (bor == false && band == false)
              finVerdict = conVerdict;
           else if (bor == true && band == false)
                finVerdict = finVerdict + conVerdict;
           else if (band == true && bor == false)
                finVerdict = finVerdict * conVerdict;
           if (Source[i] == '|')
           {
                         bor = true;
                         band = false;
                         i++;
                         if (Source[i] == '|')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
           else if (Source[i] == '&')
           {
                         bor = false;
                         band = true;
                         i++;
                         if (Source[i] == '&')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
     }
     if (finVerdict == 0)
        cond_verdict = false;
     else
        cond_verdict = true;
     if (cond_verdict == false)
     {
            GET_CONTENTS(s1,s2,*counter);
           (*counter)+=2;
           
           GET_FIRST_TOKEN(s2,interpreter_candidate);
          // printf ("New interpreter candidate: %s\n",interpreter_candidate);
           if (strcmp(interpreter_candidate,"else_if_optr") == 0)
           {
                       GET_CONTENTS(s1,s2,*counter);
                       (*counter)+=2;
           
                       GET_FIRST_TOKEN(s2,interpreter_candidate);
           }
           if (strcmp(interpreter_candidate,"op_brace") == 0)
           {
                 int numBraces = 1;
                 nEnd = *counter;
                 while (numBraces > 0)
                 {
                    GET_CONTENTS(s1,s2,nEnd);
                    nEnd += 2;
           
                    GET_FIRST_TOKEN(s2,interpreter_candidate);
              //      printf ("%s : %d\n",interpreter_candidate,numBraces);
                    if (strcmp(interpreter_candidate,"op_brace") == 0)
                       numBraces++;
                    else if (strcmp(interpreter_candidate,"cl_brace") == 0)
                       numBraces--;
                 }
                 
                 *counter = start;
                 while (*counter < nEnd)
                 {
                       GET_CONTENTS(s1,s2,*counter);
                       (*counter)+=2;
                       
                       GET_FIRST_TOKEN(s2,interpreter_candidate);
                 }
                 GET_CONTENTS(s1,s2,*counter);
                 GET_FIRST_TOKEN(s2,interpreter_candidate);
              //   printf ("Interpreter candidte: %s\n",interpreter_candidate);
                 if (strcmp("endif",interpreter_candidate) == 0)
                 {
                 }
                 else if (strcmp("else_if_optr",interpreter_candidate) == 0)
                 {
                    INTERPRET_IF_STATEMENT(s1,counter);
                 }
                 else if (strcmp("else_optr",interpreter_candidate) == 0)
                 {
                      start = *counter;
                      numBraces = 0;
                      nEnd = *counter;
                      do
                      {
                            GET_CONTENTS(s1,s2,nEnd);
                            nEnd += 2;
           
                            GET_FIRST_TOKEN(s2,interpreter_candidate);
                        //    printf ("%s\n",interpreter_candidate);
                            if (strcmp(interpreter_candidate,"else_optr") == 0)
                            {
                                                                          GET_CONTENTS(s1,s2,nEnd);
                                                                          nEnd += 2;
           
                                                                          GET_FIRST_TOKEN(s2,interpreter_candidate);
                            }
                            if (strcmp(interpreter_candidate,"op_brace") == 0)
                               numBraces++;
                            else if (strcmp(interpreter_candidate,"cl_brace") == 0)
                               numBraces--;
                      } while (numBraces > 0);
                      
                      *counter = start;
                      while (*counter < nEnd)
                      {
                          // printf ("Starting to interpret: \n");
                           GET_CONTENTS(s1,s2,*counter);
                           (*counter)+=2;
                           
                          GET_FIRST_TOKEN(s2,interpreter_candidate);
                         //  printf ("%s\n",interpreter_candidate);
                           if (strcmp(interpreter_candidate,"write_optr") == 0)
                           {
                              INTERPRET_WRITE(s1);
                           }
                           else if (strcmp(interpreter_candidate,"read_optr") == 0)
                           {
                              INTERPRET_READ(s1);
                           }
                           else if (strcmp(interpreter_candidate,"int_optr") == 0)
                           {
                              INTERPRET_INT_IDENTIFY(s1);
                           }
                           else if (strcmp(interpreter_candidate,"char_optr") == 0)
                           {
                                INTERPRET_CHAR_IDENTIFY(s1);
                           }
                           else if (strcmp(interpreter_candidate,"float_optr") == 0)
                           {
                              INTERPRET_FLOAT_IDENTIFY(s1);
                           }
                           else if (strcmp(interpreter_candidate,"string_optr") == 0)
                           {
                              INTERPRET_STRING_IDENTIFY(s1);
                           }
                           else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
                           {
                               INTERPRET_ASSIGN_STATEMENT(s1);
                           }
                           else if (strcmp(interpreter_candidate,"if_optr") == 0)
                           {
                                INTERPRET_IF_STATEMENT(s1,counter);
                           }
                           else if (strcmp(interpreter_candidate,"while_optr") == 0)
                           {
                                INTERPRET_WHILE_STATEMENT(s1,counter);
                           }
                           else if (strcmp(interpreter_candidate,"do_while_optr") == 0)
                           {
                                INTERPRET_DO_WHILE_STATEMENT(s1,counter);
                           }
                           else if (strcmp(interpreter_candidate,"for_optr") == 0)
                           {
                                INTERPRET_FOR_STATEMENT(s1,counter);
                           }
                           else if (strcmp(interpreter_candidate,"FUNCTION_CALL") == 0)
                           {
                                CALL_FUNCTION(s1);
                           }
                           else if (strcmp(interpreter_candidate,"assign_optr") == 0)
                           {
                                INTERPRET_ASSIGN_FUNCTION_STATEMENT(s1);
                           }
                           else if (strcmp(interpreter_candidate,"set_array") == 0)
                           {
                                 INTERPRET_ARRAY_ASSIGNMENT(s1);
                           }
                           else if (strcmp(interpreter_candidate,"get_array") == 0)
                           {
                                INTERPRET_ARRAY_GET(s1);
                           }
                       }
                     bool bEnd = false;
                     while (!bEnd)
                     {
                           GET_CONTENTS(s1,s2,*counter);
                           (*counter)+=2;
                           
                           GET_FIRST_TOKEN(s2,interpreter_candidate);
                           if (strcmp(interpreter_candidate,"endif") == 0)
                              bEnd = true;
                     }
                 }
             //    printf ("%s\n",s1);
           }
           else
           {
           }
     }
     else if (cond_verdict == true)
     {
           GET_CONTENTS(s1,s2,*counter);
           (*counter)+=2;
           
           GET_FIRST_TOKEN(s2,interpreter_candidate);
         //  printf ("Candidate: %s\n",interpreter_candidate);
           if (strcmp(interpreter_candidate,"else_if_optr") == 0)
           {
                       GET_CONTENTS(s1,s2,*counter);
                       (*counter)+=2;
           
                       GET_FIRST_TOKEN(s2,interpreter_candidate);
           }
           if (strcmp(interpreter_candidate,"op_brace") == 0)
           {
                 int numBraces = 1;
                 nEnd = *counter;
                 while (numBraces > 0)
                 {
                    GET_CONTENTS(s1,s2,nEnd);
                    nEnd += 2;
           
                    GET_FIRST_TOKEN(s2,interpreter_candidate);
              //      printf ("%s : %d\n",interpreter_candidate,numBraces);
                    if (strcmp(interpreter_candidate,"op_brace") == 0)
                       numBraces++;
                    else if (strcmp(interpreter_candidate,"cl_brace") == 0)
                       numBraces--;
                 }
                 
                 *counter = start;
                 while (*counter < nEnd)
                 {
                  //     printf ("Starting to interpret: \n");
                       GET_CONTENTS(s1,s2,*counter);
                       (*counter)+=2;
                       
                       GET_FIRST_TOKEN(s2,interpreter_candidate);
                  //     printf ("%s\n",interpreter_candidate);
                       if (strcmp(interpreter_candidate,"write_optr") == 0)
                       {
                          INTERPRET_WRITE(s1);
                       }
                       else if (strcmp(interpreter_candidate,"read_optr") == 0)
                       {
                          INTERPRET_READ(s1);
                       }
                       else if (strcmp(interpreter_candidate,"int_optr") == 0)
                       {
                          INTERPRET_INT_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"char_optr") == 0)
                       {
                            INTERPRET_CHAR_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"float_optr") == 0)
                       {
                          INTERPRET_FLOAT_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"string_optr") == 0)
                       {
                          INTERPRET_STRING_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
                       {
                           INTERPRET_ASSIGN_STATEMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"if_optr") == 0)
                       {
                            INTERPRET_IF_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"while_optr") == 0)
                       {
                            INTERPRET_WHILE_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"do_while_optr") == 0)
                       {
                            INTERPRET_DO_WHILE_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"for_optr") == 0)
                       {
                            INTERPRET_FOR_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"FUNCTION_CALL") == 0)
                       {
                            CALL_FUNCTION(s1);
                       }
                       else if (strcmp(interpreter_candidate,"assign_optr") == 0)
                       {
                                  INTERPRET_ASSIGN_FUNCTION_STATEMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"set_array") == 0)
                       {
                                 INTERPRET_ARRAY_ASSIGNMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"get_array") == 0)
                       {
                                INTERPRET_ARRAY_GET(s1);
                       }
                 }
                 bool bEnd = false;
                 while (!bEnd)
                 {
                       GET_CONTENTS(s1,s2,*counter);
                       (*counter)+=2;
                       
                       GET_FIRST_TOKEN(s2,interpreter_candidate);
                       if (strcmp(interpreter_candidate,"endif") == 0)
                          bEnd = true;
                 }
           }
           else
           {
               if (strcmp(interpreter_candidate,"write_optr") == 0)
               {
                  INTERPRET_WRITE(s1);
               }
               else if (strcmp(interpreter_candidate,"read_optr") == 0)
               {
                  INTERPRET_READ(s1);
               }
               else if (strcmp(interpreter_candidate,"int_optr") == 0)
               {
                  INTERPRET_INT_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"float_optr") == 0)
               {
                  INTERPRET_FLOAT_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"string_optr") == 0)
               {
                  INTERPRET_STRING_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
               {
                   INTERPRET_ASSIGN_STATEMENT(s1);
               }
               else if (strcmp(interpreter_candidate,"if_optr") == 0)
               {
                    INTERPRET_IF_STATEMENT(s1,counter);
                    //(*counter)--;
               }
               else if (strcmp(interpreter_candidate,"FUNCTION_CALL") == 0)
               {
                    CALL_FUNCTION(s1);
               }
           }
     }
     
//     printf ("if verdict: true\n");
     else
     {
         (*counter)+=2;
     }
//     printf ("if verdict: false\n");
     
}

void INTERPRET_IF_STATEMENT_FUNCTION(String Source,int *counter,int index_func,int VARIABLES_START,bool *hasReturned)
{
     String empty;
     String value;
     String cond1;
     String op;
     String cond2;
     String interpreter_candidate;
     String s1,s2;
     
     bool end = false;
     bool end2 = false;
     bool cond_verdict;
     
     int conVerdict = 0;
     int finVerdict = 0;
     
     bool bor = false;
     bool band = false;
     int nSum = 0,nEnd;
     int start = *counter;
     
     strcpy(empty,"");
     strcpy(value,"");
     int i = 0;
     int empty_index = 0;
     while (Source[i] != '(')
           i++;
     i++;
 //    printf ("************The source: %s\n",Source);
  //   printf ("************The source 2: %s\n",FUNCTIONS[index_func].statements[(*counter)-1]);
     while (!end)
     {
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != '>' && Source[i] != '<' && Source[i] != '=')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond1);
           empty_index = 0;
           if (Source[i] == '>' || Source[i] == '<' || Source[i] == '=')
           {
                empty[empty_index] = Source[i];
                i++;
                empty_index++;
                if (Source[i] == '=')
                {
                              empty[empty_index] = Source[i];
                              i++;
                              empty_index++;
                }   
                empty[empty_index] = '\0';
                strcpy(op,empty);
                if (Source[i] == ' ')
                   i++;      
           }
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != ')' && Source[i] != '|' && Source[i] != '&')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond2);
           if (Source[i] == ')')
              end = true;
           
           if (strcmp(op,">") == 0)
           {
              if (atoi(cond1) - atoi(cond2) > 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;;
           }
           else if (strcmp (op,"<") == 0)
           {
              if (atoi(cond1) - atoi(cond2) < 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,">=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) >= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"<=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) <= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"==") == 0)
           {
              if (atoi(cond1) - atoi(cond2) == 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           
           if (bor == false && band == false)
              finVerdict = conVerdict;
           else if (bor == true && band == false)
                finVerdict = finVerdict + conVerdict;
           else if (band == true && bor == false)
                finVerdict = finVerdict * conVerdict;
           if (Source[i] == '|')
           {
                         bor = true;
                         band = false;
                         i++;
                         if (Source[i] == '|')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
           else if (Source[i] == '&')
           {
                         bor = false;
                         band = true;
                         i++;
                         if (Source[i] == '&')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
     }
     if (finVerdict == 0)
        cond_verdict = false;
     else
        cond_verdict = true;
     if (cond_verdict == false)
     {
           strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
           (*counter)++;
           strcpy(s2,FUNCTIONS[index_func].statements[*counter]);
           (*counter)++;
           
           GET_FIRST_TOKEN(s2,interpreter_candidate);
          // printf ("New interpreter candidate: %s\n",interpreter_candidate);
           if (strcmp(interpreter_candidate,"else_if_optr") == 0)
           {
                                  strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
                                   (*counter)++;
                                   strcpy(s2,FUNCTIONS[index_func].statements[*counter]);
                                   (*counter)++;
           
                       GET_FIRST_TOKEN(s2,interpreter_candidate);
           }
           if (strcmp(interpreter_candidate,"op_brace") == 0)
           {
                 int numBraces = 1;
                 nEnd = *counter;
                 while (numBraces > 0)
                 {
                               strcpy(s1,FUNCTIONS[index_func].statements[nEnd]);
                               nEnd++;
                               strcpy(s2,FUNCTIONS[index_func].statements[nEnd]);
                               nEnd++;
           
                    GET_FIRST_TOKEN(s2,interpreter_candidate);
              //      printf ("%s : %d\n",interpreter_candidate,numBraces);
                    if (strcmp(interpreter_candidate,"op_brace") == 0)
                       numBraces++;
                    else if (strcmp(interpreter_candidate,"cl_brace") == 0)
                       numBraces--;
                 }
                 
                 *counter = start;
                 while (*counter < nEnd)
                 {
                                  strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
                                  (*counter)++;
                                  strcpy(s2,FUNCTIONS[index_func].statements[*counter]);
                                  (*counter)++;
                       
                                  GET_FIRST_TOKEN(s2,interpreter_candidate);
                 }
                 strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
                 strcpy(s2,FUNCTIONS[index_func].statements[(*counter)+1]);
                 GET_FIRST_TOKEN(s2,interpreter_candidate);
              //   printf ("Interpreter candidte: %s\n",interpreter_candidate);
                 if (strcmp("endif",interpreter_candidate) == 0)
                 {
                 }
                 else if (strcmp("else_if_optr",interpreter_candidate) == 0)
                 {
              //      INTERPRET_IF_STATEMENT(s1,counter);
              INTERPRET_IF_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                 }
                 else if (strcmp("else_optr",interpreter_candidate) == 0)
                 {
                      start = *counter;
                      numBraces = 0;
                      nEnd = *counter;
                      do
                      {
                                       strcpy(s1,FUNCTIONS[index_func].statements[nEnd]);
                                       nEnd++;
                                       strcpy(s2,FUNCTIONS[index_func].statements[nEnd]);
                                       nEnd++;
           
                            GET_FIRST_TOKEN(s2,interpreter_candidate);
                     //       printf ("%s\n",interpreter_candidate);
                            if (strcmp(interpreter_candidate,"else_optr") == 0)
                            {
                                       strcpy(s1,FUNCTIONS[index_func].statements[nEnd]);
                                       nEnd++;
                                       strcpy(s2,FUNCTIONS[index_func].statements[nEnd]);
                                       nEnd++;
           
                                       GET_FIRST_TOKEN(s2,interpreter_candidate);
                            }
                            if (strcmp(interpreter_candidate,"op_brace") == 0)
                               numBraces++;
                            else if (strcmp(interpreter_candidate,"cl_brace") == 0)
                               numBraces--;
                   //         printf ("%d\n",numBraces);
                      } while (numBraces > 0);
                      
                      *counter = start;
                      while (*counter < nEnd && *hasReturned == false)
                      {
                          // printf ("Starting to interpret: \n");
                          strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
                          (*counter)++;
                          strcpy(s2,FUNCTIONS[index_func].statements[*counter]);
                          (*counter)++;
                           
                          GET_FIRST_TOKEN(s2,interpreter_candidate);
                         //  printf ("%s\n",interpreter_candidate);
                           if (strcmp(interpreter_candidate,"write_optr") == 0)
                           {
                              INTERPRET_WRITE(s1);
                           }
                           else if (strcmp(interpreter_candidate,"read_optr") == 0)
                           {
                              INTERPRET_READ(s1);
                           }
                           else if (strcmp(interpreter_candidate,"int_optr") == 0)
                           {
                              INTERPRET_INT_IDENTIFY(s1);
                           }
                           else if (strcmp(interpreter_candidate,"char_optr") == 0)
                           {
                                INTERPRET_CHAR_IDENTIFY(s1);
                           }
                           else if (strcmp(interpreter_candidate,"float_optr") == 0)
                           {
                              INTERPRET_FLOAT_IDENTIFY(s1);
                           }
                           else if (strcmp(interpreter_candidate,"string_optr") == 0)
                           {
                              INTERPRET_STRING_IDENTIFY(s1);
                           }
                           else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
                           {
                               INTERPRET_ASSIGN_STATEMENT(s1);
                           }
                           else if (strcmp(interpreter_candidate,"if_optr") == 0)
                           {
                                INTERPRET_IF_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                           }
                           else if (strcmp(interpreter_candidate,"while_optr") == 0)
                           {
                                INTERPRET_WHILE_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                           }
                           else if (strcmp(interpreter_candidate,"do_while_optr") == 0)
                           {
                                INTERPRET_DO_WHILE_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                           }
                           else if (strcmp(interpreter_candidate,"for_optr") == 0)
                           {
                                INTERPRET_FOR_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                           }
                           else if (strcmp(interpreter_candidate,"FUNCTION_CALL") == 0)
                           {
                                CALL_FUNCTION(s1);
                           }
                           else if (strcmp(interpreter_candidate,"return_optr") == 0)
                           {
                                INTERPRET_RETURN_STATEMENT(s1,index_func,VARIABLES_START);
                                *counter = nEnd;
                                *hasReturned = true;
                                return;
                           }
                           else if (strcmp(interpreter_candidate,"assign_optr") == 0)
                           {
                                INTERPRET_ASSIGN_FUNCTION_STATEMENT(s1);
                           }
                       else if (strcmp(interpreter_candidate,"set_array") == 0)
                       {
                                 INTERPRET_ARRAY_ASSIGNMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"get_array") == 0)
                       {
                                INTERPRET_ARRAY_GET(s1);
                       }
                           
                       }
                     bool bEnd = false;
                //     printf ("Ended\n");
                     while (!bEnd)
                     {
                           strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
                           (*counter)++;
                           strcpy(s2,FUNCTIONS[index_func].statements[*counter]);
                           (*counter)++;
                           
                           GET_FIRST_TOKEN(s2,interpreter_candidate);
                          // printf ("Before ending: %s\n",interpreter_candidate);
                           if (strcmp(interpreter_candidate,"endif") == 0)
                              bEnd = true;
                     }
                   //   printf ("Ended 2\n");
                 }
             //    printf ("%s\n",s1);
           }
           else
           {
           }
     }
     else if (cond_verdict == true)
     {
          strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
          (*counter)++;
          strcpy(s2,FUNCTIONS[index_func].statements[*counter]);
          (*counter)++;
           
           GET_FIRST_TOKEN(s2,interpreter_candidate);
   //        printf ("The string: %s\n",s1);
   //        printf ("Candidate: %s\n",interpreter_candidate);
           if (strcmp(interpreter_candidate,"else_if_optr") == 0)
           {
                           strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
                           (*counter)++;
                           strcpy(s2,FUNCTIONS[index_func].statements[*counter]);
                           (*counter)++;
           
                           GET_FIRST_TOKEN(s2,interpreter_candidate);
           }
           if (strcmp(interpreter_candidate,"op_brace") == 0)
           {
                 int numBraces = 1;
                 nEnd = *counter;
                 while (numBraces > 0)
                 {
                                  strcpy(s1,FUNCTIONS[index_func].statements[nEnd]);
                                  nEnd++;
                                  strcpy(s2,FUNCTIONS[index_func].statements[nEnd]);
                                  nEnd++;
           
                    GET_FIRST_TOKEN(s2,interpreter_candidate);
                ///    printf ("%s : %d\n",interpreter_candidate,numBraces);
                    if (strcmp(interpreter_candidate,"op_brace") == 0)
                       numBraces++;
                    else if (strcmp(interpreter_candidate,"cl_brace") == 0)
                       numBraces--;
              //      printf ("%s : %d\n",interpreter_candidate,numBraces);
                 }
                 
                 *counter = start;
                 while (*counter < nEnd && *hasReturned == false)
                 {
                  //     printf ("Starting to interpret: \n");
                       strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
                       (*counter)++;
                       strcpy(s2,FUNCTIONS[index_func].statements[*counter]);
                       (*counter)++;
                       
                       GET_FIRST_TOKEN(s2,interpreter_candidate);
                  //     printf ("Interpreting: %s\n",interpreter_candidate);
                       if (strcmp(interpreter_candidate,"write_optr") == 0)
                       {
                          INTERPRET_WRITE(s1);
                       }
                       else if (strcmp(interpreter_candidate,"read_optr") == 0)
                       {
                          INTERPRET_READ(s1);
                       }
                       else if (strcmp(interpreter_candidate,"int_optr") == 0)
                       {
                          INTERPRET_INT_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"char_optr") == 0)
                       {
                            INTERPRET_CHAR_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"float_optr") == 0)
                       {
                          INTERPRET_FLOAT_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"string_optr") == 0)
                       {
                          INTERPRET_STRING_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
                       {
                           INTERPRET_ASSIGN_STATEMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"if_optr") == 0)
                       {
                          //  printf ("Interpreting if statement!\n");
                   //         printf ("Statement to be passed: ",
                            INTERPRET_IF_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                       }
                       else if (strcmp(interpreter_candidate,"while_optr") == 0)
                       {
                            INTERPRET_WHILE_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                       }
                       else if (strcmp(interpreter_candidate,"do_while_optr") == 0)
                       {
                            INTERPRET_DO_WHILE_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                       }
                       else if (strcmp(interpreter_candidate,"for_optr") == 0)
                       {
                            INTERPRET_FOR_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                       }
                       else if (strcmp(interpreter_candidate,"FUNCTION_CALL") == 0)
                       {
                            CALL_FUNCTION(s1);
                       }
                       else if (strcmp(interpreter_candidate,"return_optr") == 0)
                       {
                                INTERPRET_RETURN_STATEMENT(s1,index_func,VARIABLES_START);
                                *counter = nEnd;
                                *hasReturned = true;
                                return;
                       }
                       else if (strcmp(interpreter_candidate,"set_array") == 0)
                       {
                                 INTERPRET_ARRAY_ASSIGNMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"get_array") == 0)
                       {
                                INTERPRET_ARRAY_GET(s1);
                       }
                                  else if (strcmp(interpreter_candidate,"assign_optr") == 0)
           {
                INTERPRET_ASSIGN_FUNCTION_STATEMENT(s1);
           }
                 }
                 
                 bool bEnd = false;
                 while (!bEnd)
                 {
                              strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
                              (*counter)++;
                              strcpy(s2,FUNCTIONS[index_func].statements[*counter]);
                              (*counter)++;
                       
                       GET_FIRST_TOKEN(s2,interpreter_candidate);
                       if (strcmp(interpreter_candidate,"endif") == 0)
                          bEnd = true;
                 }
           }
           else
           {
               if (strcmp(interpreter_candidate,"write_optr") == 0)
               {
                  INTERPRET_WRITE(s1);
               }
               else if (strcmp(interpreter_candidate,"read_optr") == 0)
               {
                  INTERPRET_READ(s1);
               }
               else if (strcmp(interpreter_candidate,"int_optr") == 0)
               {
                  INTERPRET_INT_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"float_optr") == 0)
               {
                  INTERPRET_FLOAT_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"string_optr") == 0)
               {
                  INTERPRET_STRING_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
               {
                   INTERPRET_ASSIGN_STATEMENT(s1);
               }
               else if (strcmp(interpreter_candidate,"if_optr") == 0)
               {
                    INTERPRET_IF_STATEMENT(s1,counter);
                    //(*counter)--;
               }
               else if (strcmp(interpreter_candidate,"FUNCTION_CALL") == 0)
               {
                    CALL_FUNCTION(s1);
               }
           }
     }
     
//     printf ("if verdict: true\n");
//     printf ("if verdict: false\n");
     
}

void INTERPRET_WHILE_STATEMENT(String Source,int *counter)
{
     String empty;
     String value;
     String cond1;
     String op;
     String cond2;
     String interpreter_candidate;
     String s1,s2;
     
     int start = *counter;
     int nEnd;
     
     bool end = false;
     bool end2 = false;
     bool cond_verdict;
     int nSum = 0;
     
     int finVerdict = 0;
     int conVerdict = 0;
     
     bool bor = false;
     bool band = false;
     strcpy(empty,"");
     strcpy(value,"");
     int i = 0;
     int empty_index = 0;
     while (Source[i] != '(')
           i++;
     i++;
     while (!end)
     {
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != '>' && Source[i] != '<' && Source[i] != '=')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond1);
           empty_index = 0;
           if (Source[i] == '>' || Source[i] == '<' || Source[i] == '=')
           {
                empty[empty_index] = Source[i];
                i++;
                empty_index++;
                if (Source[i] == '=')
                {
                              empty[empty_index] = Source[i];
                              i++;
                              empty_index++;
                }   
                empty[empty_index] = '\0';
                strcpy(op,empty);
                if (Source[i] == ' ')
                   i++;      
           }
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != ')' && Source[i] != '|' && Source[i] != '&')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond2);
           if (Source[i] == ')')
              end = true;
           
           if (strcmp(op,">") == 0)
           {
              if (atoi(cond1) - atoi(cond2) > 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;;
           }
           else if (strcmp (op,"<") == 0)
           {
              if (atoi(cond1) - atoi(cond2) < 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,">=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) >= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"<=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) <= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"==") == 0)
           {
              if (atoi(cond1) - atoi(cond2) == 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
        //   printf ("Condition evaluated: %d\n",conVerdict);
           if (bor == false && band == false)
               finVerdict = conVerdict;
           else if (bor == true && band == false)
                finVerdict = finVerdict + conVerdict;
           else if (band == true && bor == false)
                finVerdict = finVerdict * conVerdict;
           if (Source[i] == '|')
           {
                         bor = true;
                         band = false;
                         i++;
                         if (Source[i] == '|')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
           else if (Source[i] == '&')
           {
                         bor = false;
                         band = true;
                         i++;
                         if (Source[i] == '&')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
      //     printf ("Evaluated: %d\n",finVerdict);
     }
     if (finVerdict == 0)
        cond_verdict = false;
     else
        cond_verdict = true;
     if (cond_verdict == false)
     {
            GET_CONTENTS(s1,s2,*counter);
           (*counter)+=2;
           
           GET_FIRST_TOKEN(s2,interpreter_candidate);
           if (strcmp(interpreter_candidate,"op_brace") == 0)
           {
                 int numBraces = 1;
                 nEnd = *counter;
                 while (numBraces > 0)
                 {
                    GET_CONTENTS(s1,s2,nEnd);
                    nEnd += 2;
           
                    GET_FIRST_TOKEN(s2,interpreter_candidate);
              //      printf ("%s : %d\n",interpreter_candidate,numBraces);
                    if (strcmp(interpreter_candidate,"op_brace") == 0)
                       numBraces++;
                    else if (strcmp(interpreter_candidate,"cl_brace") == 0)
                       numBraces--;
                 }
                 
                 *counter = start;
                 while (*counter < nEnd)
                 {
                  //     printf ("Starting to interpret: \n");
                       GET_CONTENTS(s1,s2,*counter);
                       (*counter)+=2;
                       
                       GET_FIRST_TOKEN(s2,interpreter_candidate);
                  //     printf ("%s\n",interpreter_candidate);
                 }
           }
           else
           {
           }
     }
     else
     while (cond_verdict == true)
     {
           GET_CONTENTS(s1,s2,*counter);
           (*counter)+=2;
           
           GET_FIRST_TOKEN(s2,interpreter_candidate);
        //   printf ("%s\n",interpreter_candidate);
           if (strcmp(interpreter_candidate,"op_brace") == 0)
           {
                 int numBraces = 1;
                 nEnd = *counter;
                 while (numBraces > 0)
                 {
                    GET_CONTENTS(s1,s2,nEnd);
                    nEnd += 2;
           
                    GET_FIRST_TOKEN(s2,interpreter_candidate);
              //      printf ("%s : %d\n",interpreter_candidate,numBraces);
                    if (strcmp(interpreter_candidate,"op_brace") == 0)
                       numBraces++;
                    else if (strcmp(interpreter_candidate,"cl_brace") == 0)
                       numBraces--;
                 }
                 
                 *counter = start;
                 while (*counter < nEnd)
                 {
                  //     printf ("Starting to interpret: \n");
                       GET_CONTENTS(s1,s2,*counter);
                       (*counter)+=2;
                       
                       GET_FIRST_TOKEN(s2,interpreter_candidate);
                  //     printf ("%s\n",interpreter_candidate);
                       if (strcmp(interpreter_candidate,"write_optr") == 0)
                       {
                          INTERPRET_WRITE(s1);
                       }
                       else if (strcmp(interpreter_candidate,"read_optr") == 0)
                       {
                          INTERPRET_READ(s1);
                       }
                       else if (strcmp(interpreter_candidate,"int_optr") == 0)
                       {
                          INTERPRET_INT_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"char_optr") == 0)
                       {
                            INTERPRET_CHAR_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"float_optr") == 0)
                       {
                          INTERPRET_FLOAT_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"string_optr") == 0)
                       {
                          INTERPRET_STRING_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
                       {
                           INTERPRET_ASSIGN_STATEMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"if_optr") == 0)
                       {
                            INTERPRET_IF_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"while_optr") == 0)
                       {
                            INTERPRET_WHILE_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"do_while_optr") == 0)
                       {
                                INTERPRET_DO_WHILE_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"for_optr") == 0)
                       {
                                INTERPRET_FOR_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"FUNCTION_CALL") == 0)
                       {
                            CALL_FUNCTION(s1);
                       }
                       else if (strcmp(interpreter_candidate,"set_array") == 0)
                       {
                                 INTERPRET_ARRAY_ASSIGNMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"get_array") == 0)
                       {
                                INTERPRET_ARRAY_GET(s1);
                       }
                                  else if (strcmp(interpreter_candidate,"assign_optr") == 0)
           {
                INTERPRET_ASSIGN_FUNCTION_STATEMENT(s1);
           }
                 }
           }
           else
           {
               if (strcmp(interpreter_candidate,"write_optr") == 0)
               {
                  INTERPRET_WRITE(s1);
               }
               else if (strcmp(interpreter_candidate,"read_optr") == 0)
               {
                  INTERPRET_READ(s1);
               }
               else if (strcmp(interpreter_candidate,"int_optr") == 0)
               {
                  INTERPRET_INT_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"char_optr") == 0)
               {
                    INTERPRET_CHAR_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"float_optr") == 0)
               {
                  INTERPRET_FLOAT_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"string_optr") == 0)
               {
                  INTERPRET_STRING_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
               {
                   INTERPRET_ASSIGN_STATEMENT(s1);
               }
               else if (strcmp(interpreter_candidate,"if_optr") == 0)
               {
                    INTERPRET_IF_STATEMENT(s1,counter);
                    (*counter)--;
               }
           }
     
     end = false;     
     bor = false;
     band = false;
     strcpy(empty,"");
     strcpy(value,"");
     i = 0;
     empty_index = 0;
     while (Source[i] != '(')
           i++;
     i++;
      
      while (!end)
     {
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != '>' && Source[i] != '<' && Source[i] != '=')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond1);
           empty_index = 0;
           if (Source[i] == '>' || Source[i] == '<' || Source[i] == '=')
           {
                empty[empty_index] = Source[i];
                i++;
                empty_index++;
                if (Source[i] == '=')
                {
                              empty[empty_index] = Source[i];
                              i++;
                              empty_index++;
                }   
                empty[empty_index] = '\0';
                strcpy(op,empty);
                if (Source[i] == ' ')
                   i++;      
           }
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != ')' && Source[i] != '|' && Source[i] != '&')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond2);
           if (Source[i] == ')')
              end = true;
           
           if (strcmp(op,">") == 0)
           {
              if (atoi(cond1) - atoi(cond2) > 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;;
           }
           else if (strcmp (op,"<") == 0)
           {
              if (atoi(cond1) - atoi(cond2) < 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,">=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) >= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"<=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) <= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"==") == 0)
           {
              if (atoi(cond1) - atoi(cond2) == 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           
           if (bor == false && band == false)
              finVerdict = conVerdict;
           else if (bor == true && band == false)
                finVerdict = finVerdict + conVerdict;
           else if (band == true && bor == false)
                finVerdict = finVerdict * conVerdict;
           if (Source[i] == '|')
           {
                         bor = true;
                         band = false;
                         i++;
                         if (Source[i] == '|')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
           else if (Source[i] == '&')
           {
                         bor = false;
                         band = true;
                         i++;
                         if (Source[i] == '&')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
     }
     if (finVerdict == 0)
        cond_verdict = false;
     else
        cond_verdict = true;
           if (cond_verdict == true)
              *counter = start;
     }
}     

void INTERPRET_WHILE_STATEMENT_FUNCTION(String Source,int *counter,int index_func,int VARIABLES_START,bool *hasReturned)
{
     String empty;
     String value;
     String cond1;
     String op;
     String cond2;
     String interpreter_candidate;
     String s1,s2;
     
     int conVerdict;
     int finVerdict;
     
     bool band = false;
     bool bor = false;
     int start = *counter;
     int nEnd;
     
     bool end = false;
     bool end2 = false;
     bool cond_verdict;
     int nSum = 0;
     
     strcpy(empty,"");
     strcpy(value,"");
     int i = 0;
     int empty_index = 0;
     while (Source[i] != '(')
           i++;
     i++;
     
     while (!end)
     {
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != '>' && Source[i] != '<' && Source[i] != '=')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond1);
           empty_index = 0;
           if (Source[i] == '>' || Source[i] == '<' || Source[i] == '=')
           {
                empty[empty_index] = Source[i];
                i++;
                empty_index++;
                if (Source[i] == '=')
                {
                              empty[empty_index] = Source[i];
                              i++;
                              empty_index++;
                }   
                empty[empty_index] = '\0';
                strcpy(op,empty);
                if (Source[i] == ' ')
                   i++;      
           }
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != ')' && Source[i] != '|' && Source[i] != '&')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond2);
           if (Source[i] == ')')
              end = true;
           
           if (strcmp(op,">") == 0)
           {
              if (atoi(cond1) - atoi(cond2) > 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;;
           }
           else if (strcmp (op,"<") == 0)
           {
              if (atoi(cond1) - atoi(cond2) < 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,">=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) >= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"<=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) <= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"==") == 0)
           {
              if (atoi(cond1) - atoi(cond2) == 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
        //   printf ("Condition evaluated: %d\n",conVerdict);
           if (bor == false && band == false)
               finVerdict = conVerdict;
           else if (bor == true && band == false)
                finVerdict = finVerdict + conVerdict;
           else if (band == true && bor == false)
                finVerdict = finVerdict * conVerdict;
           if (Source[i] == '|')
           {
                         bor = true;
                         band = false;
                         i++;
                         if (Source[i] == '|')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
           else if (Source[i] == '&')
           {
                         bor = false;
                         band = true;
                         i++;
                         if (Source[i] == '&')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
      //     printf ("Evaluated: %d\n",finVerdict);
     }
     if (finVerdict == 0)
        cond_verdict = false;
     else
        cond_verdict = true;
   //  printf ("Finished evaluating: ");
   //  getch();
     while (cond_verdict == true)
     {
           strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
           (*counter)++;
           strcpy(s2,FUNCTIONS[index_func].statements[*counter]);
           (*counter)++;
           
           GET_FIRST_TOKEN(s2,interpreter_candidate);
        //   printf ("Interpreting: %s\n",s1);
        //   printf ("%s\n",interpreter_candidate);
           if (strcmp(interpreter_candidate,"op_brace") == 0)
           {
                 int numBraces = 1;
                 nEnd = *counter;
                 while (numBraces > 0)
                 {
                       strcpy(s1,FUNCTIONS[index_func].statements[nEnd]);
                       nEnd++;
                       strcpy(s2,FUNCTIONS[index_func].statements[nEnd]);
                       nEnd++;
           
                    GET_FIRST_TOKEN(s2,interpreter_candidate);
              //      printf ("%s : %d\n",interpreter_candidate,numBraces);
                    if (strcmp(interpreter_candidate,"op_brace") == 0)
                       numBraces++;
                    else if (strcmp(interpreter_candidate,"cl_brace") == 0)
                       numBraces--;
                 }
                 
                 *counter = start;
                 while (*counter < nEnd && *hasReturned == false)
                 {
                  //     printf ("Starting to interpret: \n");
                       strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
                       (*counter)++;
                       strcpy(s2,FUNCTIONS[index_func].statements[*counter]);
                       (*counter)++;
                       
                       GET_FIRST_TOKEN(s2,interpreter_candidate);
                  //     printf ("%s\n",interpreter_candidate);
                       if (strcmp(interpreter_candidate,"write_optr") == 0)
                       {
                          INTERPRET_WRITE(s1);
                       }
                       else if (strcmp(interpreter_candidate,"read_optr") == 0)
                       {
                          INTERPRET_READ(s1);
                       }
                       else if (strcmp(interpreter_candidate,"int_optr") == 0)
                       {
                          INTERPRET_INT_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"char_optr") == 0)
                       {
                            INTERPRET_CHAR_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"float_optr") == 0)
                       {
                          INTERPRET_FLOAT_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"string_optr") == 0)
                       {
                          INTERPRET_STRING_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
                       {
                           INTERPRET_ASSIGN_STATEMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"if_optr") == 0)
                       {
                            INTERPRET_IF_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                       }
                       else if (strcmp(interpreter_candidate,"while_optr") == 0)
                       {
                            INTERPRET_WHILE_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                       }
                       else if (strcmp(interpreter_candidate,"do_while_optr") == 0)
                       {
                            INTERPRET_DO_WHILE_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                       }
                       else if (strcmp(interpreter_candidate,"for_optr") == 0)
                       {
                            INTERPRET_FOR_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                       }
                       else if (strcmp(interpreter_candidate,"FUNCTION_CALL") == 0)
                       {
                            CALL_FUNCTION(s1);
                       }
                       else if (strcmp(interpreter_candidate,"return_optr") == 0)
                       {
                                INTERPRET_RETURN_STATEMENT(s1,index_func,VARIABLES_START);
                                *counter = nEnd;
                                *hasReturned = true;
                                return;
                       }
                       else if (strcmp(interpreter_candidate,"set_array") == 0)
                       {
                                 INTERPRET_ARRAY_ASSIGNMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"get_array") == 0)
                       {
                                INTERPRET_ARRAY_GET(s1);
                       }
                                  else if (strcmp(interpreter_candidate,"assign_optr") == 0)
           {
                INTERPRET_ASSIGN_FUNCTION_STATEMENT(s1);
           }
                 }
           }
           else
           {
               if (strcmp(interpreter_candidate,"write_optr") == 0)
               {
                  INTERPRET_WRITE(s1);
               }
               else if (strcmp(interpreter_candidate,"read_optr") == 0)
               {
                  INTERPRET_READ(s1);
               }
               else if (strcmp(interpreter_candidate,"int_optr") == 0)
               {
                  INTERPRET_INT_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"char_optr") == 0)
               {
                    INTERPRET_CHAR_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"float_optr") == 0)
               {
                  INTERPRET_FLOAT_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"string_optr") == 0)
               {
                  INTERPRET_STRING_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
               {
                   INTERPRET_ASSIGN_STATEMENT(s1);
               }
               else if (strcmp(interpreter_candidate,"if_optr") == 0)
               {
                    INTERPRET_IF_STATEMENT(s1,counter);
                    (*counter)--;
               }
               else if (strcmp(interpreter_candidate,"FUNCTION_CALL") == 0)
               {
                          CALL_FUNCTION(s1);
               }
           }
     
     end = false;     
     
     bor = false;
     band = false;
     strcpy(empty,"");
     strcpy(value,"");
     i = 0;
     empty_index = 0;
     while (Source[i] != '(')
           i++;
     i++;
      
     while (!end)
     {
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != '>' && Source[i] != '<' && Source[i] != '=')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond1);
           empty_index = 0;
           if (Source[i] == '>' || Source[i] == '<' || Source[i] == '=')
           {
                empty[empty_index] = Source[i];
                i++;
                empty_index++;
                if (Source[i] == '=')
                {
                              empty[empty_index] = Source[i];
                              i++;
                              empty_index++;
                }   
                empty[empty_index] = '\0';
                strcpy(op,empty);
                if (Source[i] == ' ')
                   i++;      
           }
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != ')' && Source[i] != '|' && Source[i] != '&')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond2);
           if (Source[i] == ')')
              end = true;
           
           if (strcmp(op,">") == 0)
           {
              if (atoi(cond1) - atoi(cond2) > 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;;
           }
           else if (strcmp (op,"<") == 0)
           {
              if (atoi(cond1) - atoi(cond2) < 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,">=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) >= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"<=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) <= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"==") == 0)
           {
              if (atoi(cond1) - atoi(cond2) == 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
        //   printf ("Condition evaluated: %d\n",conVerdict);
           if (bor == false && band == false)
               finVerdict = conVerdict;
           else if (bor == true && band == false)
                finVerdict = finVerdict + conVerdict;
           else if (band == true && bor == false)
                finVerdict = finVerdict * conVerdict;
           if (Source[i] == '|')
           {
                         bor = true;
                         band = false;
                         i++;
                         if (Source[i] == '|')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
           else if (Source[i] == '&')
           {
                         bor = false;
                         band = true;
                         i++;
                         if (Source[i] == '&')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
      //     printf ("Evaluated: %d\n",finVerdict);
     }
     if (finVerdict == 0)
        cond_verdict = false;
     else
        cond_verdict = true;
           if (cond_verdict == true)
              *counter = start;
     }
}     
//     printf ("if verdict: true\n");
   /*  else
     {
         (*counter)+=2;
     } */
//     printf ("if verdict: false\n");
     

void INTERPRET_DO_WHILE_STATEMENT(String Source,int *counter)
{
     String empty;
     String value;
     String cond1;
     String op;
     String cond2;
     String interpreter_candidate;
     String s1,s2;
     
     int start = *counter;
     int nEnd;
     
     bool band = false;
     bool bor = false;
     
     int finVerdict = 0;
     int conVerdict = 0;
     
     bool end = false;
     bool end2 = false;
     bool cond_verdict;
     int nSum = 0;
     
     strcpy(empty,"");
     strcpy(value,"");
     int i = 0;
     int empty_index = 0;
     while (Source[i] != '(')
           i++;
     i++;
        while (!end)
     {
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != '>' && Source[i] != '<' && Source[i] != '=')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond1);
           empty_index = 0;
           if (Source[i] == '>' || Source[i] == '<' || Source[i] == '=')
           {
                empty[empty_index] = Source[i];
                i++;
                empty_index++;
                if (Source[i] == '=')
                {
                              empty[empty_index] = Source[i];
                              i++;
                              empty_index++;
                }   
                empty[empty_index] = '\0';
                strcpy(op,empty);
                if (Source[i] == ' ')
                   i++;      
           }
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != ')' && Source[i] != '|' && Source[i] != '&')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond2);
           if (Source[i] == ')')
              end = true;
           
           if (strcmp(op,">") == 0)
           {
              if (atoi(cond1) - atoi(cond2) > 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;;
           }
           else if (strcmp (op,"<") == 0)
           {
              if (atoi(cond1) - atoi(cond2) < 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,">=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) >= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"<=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) <= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"==") == 0)
           {
              if (atoi(cond1) - atoi(cond2) == 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
        //   printf ("Condition evaluated: %d\n",conVerdict);
           if (bor == false && band == false)
               finVerdict = conVerdict;
           else if (bor == true && band == false)
                finVerdict = finVerdict + conVerdict;
           else if (band == true && bor == false)
                finVerdict = finVerdict * conVerdict;
           if (Source[i] == '|')
           {
                         bor = true;
                         band = false;
                         i++;
                         if (Source[i] == '|')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
           else if (Source[i] == '&')
           {
                         bor = false;
                         band = true;
                         i++;
                         if (Source[i] == '&')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
      //     printf ("Evaluated: %d\n",finVerdict);
     }
     if (finVerdict == 0)
        cond_verdict = false;
     else
        cond_verdict = true;
   //  printf ("Finished evaluating: ");
   //  getch();
     while (cond_verdict == true)
     {
           /*
           strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
           (*counter)++;
           strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
           (*counter)++;
           */
            GET_CONTENTS(s1,s2,*counter);
            (*counter)+=2;
           GET_FIRST_TOKEN(s2,interpreter_candidate);
        //   printf ("%s\n",interpreter_candidate);
           if (strcmp(interpreter_candidate,"op_brace") == 0)
           {
                 int numBraces = 1;
                 nEnd = *counter;
                 while (numBraces > 0)
                 {
                    GET_CONTENTS(s1,s2,nEnd);
                    nEnd += 2;
           
                    GET_FIRST_TOKEN(s2,interpreter_candidate);
              //      printf ("%s : %d\n",interpreter_candidate,numBraces);
                    if (strcmp(interpreter_candidate,"op_brace") == 0)
                       numBraces++;
                    else if (strcmp(interpreter_candidate,"cl_brace") == 0)
                       numBraces--;
                 }
                 
                 *counter = start;
                 while (*counter < nEnd)
                 {
                  //     printf ("Starting to interpret: \n");
                       GET_CONTENTS(s1,s2,*counter);
                       (*counter)+=2;
                       
                       GET_FIRST_TOKEN(s2,interpreter_candidate);
                  //     printf ("%s\n",interpreter_candidate);
                                             if (strcmp(interpreter_candidate,"write_optr") == 0)
                       {
                          INTERPRET_WRITE(s1);
                       }
                       else if (strcmp(interpreter_candidate,"read_optr") == 0)
                       {
                          INTERPRET_READ(s1);
                       }
                       else if (strcmp(interpreter_candidate,"int_optr") == 0)
                       {
                          INTERPRET_INT_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"char_optr") == 0)
                       {
                            INTERPRET_CHAR_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"float_optr") == 0)
                       {
                          INTERPRET_FLOAT_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"string_optr") == 0)
                       {
                          INTERPRET_STRING_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
                       {
                           INTERPRET_ASSIGN_STATEMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"if_optr") == 0)
                       {
                            INTERPRET_IF_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"while_optr") == 0)
                       {
                            INTERPRET_WHILE_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"do_while_optr") == 0)
                       {
                                INTERPRET_DO_WHILE_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"for_optr") == 0)
                       {
                                INTERPRET_FOR_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"FUNCTION_CALL") == 0)
                       {
                            CALL_FUNCTION(s1);
                       }
                       else if (strcmp(interpreter_candidate,"set_array") == 0)
                       {
                                 INTERPRET_ARRAY_ASSIGNMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"get_array") == 0)
                       {
                                INTERPRET_ARRAY_GET(s1);
                       }
                                  else if (strcmp(interpreter_candidate,"assign_optr") == 0)
           {
                INTERPRET_ASSIGN_FUNCTION_STATEMENT(s1);
           }
                 }
           }
           else
           {
               if (strcmp(interpreter_candidate,"write_optr") == 0)
               {
                  INTERPRET_WRITE(s1);
               }
               else if (strcmp(interpreter_candidate,"read_optr") == 0)
               {
                  INTERPRET_READ(s1);
               }
               else if (strcmp(interpreter_candidate,"int_optr") == 0)
               {
                  INTERPRET_INT_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"char_optr") == 0)
               {
                    INTERPRET_CHAR_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"float_optr") == 0)
               {
                  INTERPRET_FLOAT_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"string_optr") == 0)
               {
                  INTERPRET_STRING_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
               {
                   INTERPRET_ASSIGN_STATEMENT(s1);
               }
               else if (strcmp(interpreter_candidate,"if_optr") == 0)
               {
                    INTERPRET_IF_STATEMENT(s1,counter);
                    (*counter)--;
               }
           }
     
     end = false;     
     bor = false;
     band = false;
     strcpy(empty,"");
     strcpy(value,"");
     i = 0;
     empty_index = 0;
     while (Source[i] != '(')
           i++;
     i++;
      
      while (!end)
     {
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != '>' && Source[i] != '<' && Source[i] != '=')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond1);
           empty_index = 0;
           if (Source[i] == '>' || Source[i] == '<' || Source[i] == '=')
           {
                empty[empty_index] = Source[i];
                i++;
                empty_index++;
                if (Source[i] == '=')
                {
                              empty[empty_index] = Source[i];
                              i++;
                              empty_index++;
                }   
                empty[empty_index] = '\0';
                strcpy(op,empty);
                if (Source[i] == ' ')
                   i++;      
           }
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != ')' && Source[i] != '|' && Source[i] != '&')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond2);
           if (Source[i] == ')')
              end = true;
           
           if (strcmp(op,">") == 0)
           {
              if (atoi(cond1) - atoi(cond2) > 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;;
           }
           else if (strcmp (op,"<") == 0)
           {
              if (atoi(cond1) - atoi(cond2) < 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,">=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) >= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"<=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) <= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"==") == 0)
           {
              if (atoi(cond1) - atoi(cond2) == 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
        //   printf ("Condition evaluated: %d\n",conVerdict);
           if (bor == false && band == false)
               finVerdict = conVerdict;
           else if (bor == true && band == false)
                finVerdict = finVerdict + conVerdict;
           else if (band == true && bor == false)
                finVerdict = finVerdict * conVerdict;
           if (Source[i] == '|')
           {
                         bor = true;
                         band = false;
                         i++;
                         if (Source[i] == '|')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
           else if (Source[i] == '&')
           {
                         bor = false;
                         band = true;
                         i++;
                         if (Source[i] == '&')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
      //     printf ("Evaluated: %d\n",finVerdict);
     }
     if (finVerdict == 0)
        cond_verdict = false;
     else
        cond_verdict = true;
           if (cond_verdict == true)
              *counter = start;
     }
}     

void INTERPRET_DO_WHILE_STATEMENT_FUNCTION(String Source,int *counter,int index_func,int VARIABLES_START,bool *hasReturned)
{
     String empty;
     String value;
     String cond1;
     String op;
     String cond2;
     String interpreter_candidate;
     String s1,s2;
     
     int start = *counter;
     int nEnd;
     
     bool bor = false;
     bool band = false;
     
     int finVerdict = 0;
     int conVerdict = 0;
     bool end = false;
     bool end2 = false;
     bool cond_verdict;
     int nSum = 0;
     
     strcpy(empty,"");
     strcpy(value,"");
     int i = 0;
     int empty_index = 0;
     while (Source[i] != '(')
           i++;
     i++;
     
     while (!end)
     {
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != '>' && Source[i] != '<' && Source[i] != '=')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond1);
           empty_index = 0;
           if (Source[i] == '>' || Source[i] == '<' || Source[i] == '=')
           {
                empty[empty_index] = Source[i];
                i++;
                empty_index++;
                if (Source[i] == '=')
                {
                              empty[empty_index] = Source[i];
                              i++;
                              empty_index++;
                }   
                empty[empty_index] = '\0';
                strcpy(op,empty);
                if (Source[i] == ' ')
                   i++;      
           }
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != ')' && Source[i] != '|' && Source[i] != '&')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond2);
           if (Source[i] == ')')
              end = true;
           
           if (strcmp(op,">") == 0)
           {
              if (atoi(cond1) - atoi(cond2) > 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;;
           }
           else if (strcmp (op,"<") == 0)
           {
              if (atoi(cond1) - atoi(cond2) < 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,">=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) >= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"<=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) <= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"==") == 0)
           {
              if (atoi(cond1) - atoi(cond2) == 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
        //   printf ("Condition evaluated: %d\n",conVerdict);
           if (bor == false && band == false)
               finVerdict = conVerdict;
           else if (bor == true && band == false)
                finVerdict = finVerdict + conVerdict;
           else if (band == true && bor == false)
                finVerdict = finVerdict * conVerdict;
           if (Source[i] == '|')
           {
                         bor = true;
                         band = false;
                         i++;
                         if (Source[i] == '|')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
           else if (Source[i] == '&')
           {
                         bor = false;
                         band = true;
                         i++;
                         if (Source[i] == '&')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
      //     printf ("Evaluated: %d\n",finVerdict);
     }
     if (finVerdict == 0)
        cond_verdict = false;
     else
        cond_verdict = true;
   //  printf ("Finished evaluating: ");
   //  getch();
     while (cond_verdict == true)
     {
            strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
           (*counter)++;
           strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
           (*counter)++;
           
           GET_FIRST_TOKEN(s2,interpreter_candidate);
        //   printf ("%s\n",interpreter_candidate);
           if (strcmp(interpreter_candidate,"op_brace") == 0)
           {
                 int numBraces = 1;
                 nEnd = *counter;
                 while (numBraces > 0)
                 {
                     strcpy(s1,FUNCTIONS[index_func].statements[nEnd]);
                   nEnd++;
                   strcpy(s1,FUNCTIONS[index_func].statements[nEnd]);
                   nEnd++;
           
                    GET_FIRST_TOKEN(s2,interpreter_candidate);
              //      printf ("%s : %d\n",interpreter_candidate,numBraces);
                    if (strcmp(interpreter_candidate,"op_brace") == 0)
                       numBraces++;
                    else if (strcmp(interpreter_candidate,"cl_brace") == 0)
                       numBraces--;
                 }
                 
                 *counter = start;
                 while (*counter < nEnd && *hasReturned == false)
                 {
                  //     printf ("Starting to interpret: \n");
                                   strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
           (*counter)++;
           strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
           (*counter)++;
                       GET_FIRST_TOKEN(s2,interpreter_candidate);
                  //     printf ("%s\n",interpreter_candidate);
                       if (strcmp(interpreter_candidate,"write_optr") == 0)
                       {
                          INTERPRET_WRITE(s1);
                       }
                       else if (strcmp(interpreter_candidate,"read_optr") == 0)
                       {
                          INTERPRET_READ(s1);
                       }
                       else if (strcmp(interpreter_candidate,"int_optr") == 0)
                       {
                          INTERPRET_INT_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"char_optr") == 0)
                       {
                            INTERPRET_CHAR_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"float_optr") == 0)
                       {
                          INTERPRET_FLOAT_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"string_optr") == 0)
                       {
                          INTERPRET_STRING_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
                       {
                           INTERPRET_ASSIGN_STATEMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"if_optr") == 0)
                       {
                            INTERPRET_IF_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                       }
                       else if (strcmp(interpreter_candidate,"while_optr") == 0)
                       {
                            INTERPRET_WHILE_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                       }
                       else if (strcmp(interpreter_candidate,"do_while_optr") == 0)
                       {
                                INTERPRET_DO_WHILE_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                       }
                       else if (strcmp(interpreter_candidate,"for_optr") == 0)
                       {
                                INTERPRET_FOR_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                       }
                       else if (strcmp(interpreter_candidate,"FUNCTION_CALL") == 0)
                       {
                            CALL_FUNCTION(s1);
                       }
                       else if (strcmp(interpreter_candidate,"return_optr") == 0)
                       {
                             INTERPRET_RETURN_STATEMENT(s1,index_func,VARIABLES_START);
                             *counter = nEnd;
                             *hasReturned = false;
                             return;
                       }
                       else if (strcmp(interpreter_candidate,"set_array") == 0)
                       {
                                 INTERPRET_ARRAY_ASSIGNMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"get_array") == 0)
                       {
                                INTERPRET_ARRAY_GET(s1);
                       }
                                  else if (strcmp(interpreter_candidate,"assign_optr") == 0)
           {
                INTERPRET_ASSIGN_FUNCTION_STATEMENT(s1);
           }
                 }
           }
           else
           {
               if (strcmp(interpreter_candidate,"write_optr") == 0)
               {
                  INTERPRET_WRITE(s1);
               }
               else if (strcmp(interpreter_candidate,"read_optr") == 0)
               {
                  INTERPRET_READ(s1);
               }
               else if (strcmp(interpreter_candidate,"int_optr") == 0)
               {
                  INTERPRET_INT_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"char_optr") == 0)
               {
                    INTERPRET_CHAR_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"float_optr") == 0)
               {
                  INTERPRET_FLOAT_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"string_optr") == 0)
               {
                  INTERPRET_STRING_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
               {
                   INTERPRET_ASSIGN_STATEMENT(s1);
               }
               else if (strcmp(interpreter_candidate,"if_optr") == 0)
               {
                    INTERPRET_IF_STATEMENT(s1,counter);
                    (*counter)--;
               }
           }
     
     end = false;     
     bor = false;
     band = false;
     strcpy(empty,"");
     strcpy(value,"");
     i = 0;
     empty_index = 0;
     while (Source[i] != '(')
           i++;
     i++;
      
          while (!end)
     {
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != '>' && Source[i] != '<' && Source[i] != '=')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond1);
           empty_index = 0;
           if (Source[i] == '>' || Source[i] == '<' || Source[i] == '=')
           {
                empty[empty_index] = Source[i];
                i++;
                empty_index++;
                if (Source[i] == '=')
                {
                              empty[empty_index] = Source[i];
                              i++;
                              empty_index++;
                }   
                empty[empty_index] = '\0';
                strcpy(op,empty);
                if (Source[i] == ' ')
                   i++;      
           }
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (Source[i] != ')' && Source[i] != '|' && Source[i] != '&')
                 {
                               empty[empty_index] = Source[i];
                               empty_index++;
                               i++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond2);
           if (Source[i] == ')')
              end = true;
           
           if (strcmp(op,">") == 0)
           {
              if (atoi(cond1) - atoi(cond2) > 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;;
           }
           else if (strcmp (op,"<") == 0)
           {
              if (atoi(cond1) - atoi(cond2) < 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,">=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) >= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"<=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) <= 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
           else if (strcmp (op,"==") == 0)
           {
              if (atoi(cond1) - atoi(cond2) == 0)
                 conVerdict = 1;
              else
                 conVerdict = 0;
           }
        //   printf ("Condition evaluated: %d\n",conVerdict);
           if (bor == false && band == false)
               finVerdict = conVerdict;
           else if (bor == true && band == false)
                finVerdict = finVerdict + conVerdict;
           else if (band == true && bor == false)
                finVerdict = finVerdict * conVerdict;
           if (Source[i] == '|')
           {
                         bor = true;
                         band = false;
                         i++;
                         if (Source[i] == '|')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
           else if (Source[i] == '&')
           {
                         bor = false;
                         band = true;
                         i++;
                         if (Source[i] == '&')
                            i++;
                         if (Source[i] == ' ')
                            i++;
           }
      //     printf ("Evaluated: %d\n",finVerdict);
     }
     if (finVerdict == 0)
        cond_verdict = false;
     else
        cond_verdict = true;
           if (cond_verdict == true)
              *counter = start;
     }
}     

void INTERPRET_FOR_STATEMENT(String Source,int *counter)
{
     String empty;
     String value;
     String cond1;
     String op;
     String cond2;
     String interpreter_candidate;
     String s1,s2;
     String condition;
     String var_loop;
     String var_loop_value;
     
     String update;
     int update_index;
     
     int start = *counter;
     int nEnd;
     int start_condition;
     bool end = false;
     bool end2 = false;
     bool cond_verdict;
     int nSum = 0;
     int cond_index;
     
     strcpy(empty,"");
     strcpy(value,"");
     int i = 0;
     int empty_index = 0;
     while (Source[i] != '(')
           i++;
     i++;
     
     while (Source[i] != ' ' && Source[i] != '=')
     {
           var_loop[empty_index] = Source[i];
           empty_index++;
           i++;
     }
     var_loop[empty_index] = '\0';
     empty_index = 0;
     if (Source[i] == ' ')
        i++;
     if (Source[i] == '=')
        i++;
     if (Source[i] == ' ')
        i++;
     while (Source[i] != ';' && Source[i] != ' ')
     {
           var_loop_value[empty_index] = Source[i];
           empty_index++;
           i++;
     }
     var_loop_value[empty_index] = '\0';
     empty_index = 0;
     UPDATE_VARIABLE(var_loop,var_loop_value);
     if (Source[i] == ' ')
        i++;
     if (Source[i] == ';')
        i++;
     if (Source[i] == ' ')
        i++;
     int temp_i = i;
     while (Source[temp_i] != ' ')
           temp_i++;
     temp_i++;
     while (Source[temp_i] != ';')
     {
           condition[cond_index] = Source[temp_i];
           temp_i++;
           cond_index++;
     }
     condition[cond_index] = ';';
     cond_index++;
     condition[cond_index] = '\0'; 
  //   printf ("Condition detected: %s\n",condition);
     
     update_index = 0;
     
     while (Source[temp_i] != ' ')
           temp_i++;
     temp_i++;
     while (Source[temp_i] != ';')
     {
           update[update_index] = Source[temp_i];
           temp_i++;
           update_index++;
     }
     
     update[update_index] = ';';
     update_index++;
     update[update_index] = '\0';
     
   //  printf ("Assignment equation: %s\n",update);
     cond_index = 0;  
     while (!end)
     {
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (condition[cond_index] != '>' && condition[cond_index] != '<' && condition[cond_index] != '=')
                 {
                               empty[empty_index] = condition[cond_index];
                               empty_index++;
                               cond_index++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond1);
           empty_index = 0;
           if (condition[cond_index] == '>' || condition[cond_index] == '<' || condition[cond_index] == '=')
           {
                empty[empty_index] = condition[cond_index];
                cond_index++;
                empty_index++;
                if (condition[cond_index] == '=')
                {
                              empty[empty_index] = condition[cond_index];
                              cond_index++;
                              empty_index++;
                }   
                empty[empty_index] = '\0';
                strcpy(op,empty);
                if (condition[cond_index] == ' ')
                   cond_index++;  
           }
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (condition[cond_index] != ';')
                 {
                               empty[empty_index] = condition[cond_index];
                               empty_index++;
                               cond_index++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond2);
           end = true;
           
         //  printf ("%s ? %s\n",cond1,cond2);
           if (strcmp(op,">") == 0)
           {
              if (atoi(cond1) - atoi(cond2) > 0)
                 cond_verdict = true;
              else
                 cond_verdict = false;
           }
           else if (strcmp (op,"<") == 0)
           {
              if (atoi(cond1) - atoi(cond2) < 0)
                 cond_verdict = true;
              else
                 cond_verdict = false;
           }
           else if (strcmp (op,">=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) >= 0)
                 cond_verdict = true;
              else
                 cond_verdict = false;
           }
           else if (strcmp (op,"<=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) <= 0)
                 cond_verdict = true;
              else
                 cond_verdict = false;
           }
           else if (strcmp (op,"==") == 0)
           {
              if (atoi(cond1) - atoi(cond2) == 0)
                 cond_verdict = true;
              else
                 cond_verdict = false;
           }
           
      /*     if (cond_verdict == true)
              printf ("TRUE\n");
           else
              printf ("FALSE\n");*/
     }
     
     if (cond_verdict == false)
     {
            GET_CONTENTS(s1,s2,*counter);
           (*counter)+=2;
           
           GET_FIRST_TOKEN(s2,interpreter_candidate);
           if (strcmp(interpreter_candidate,"op_brace") == 0)
           {
                 int numBraces = 1;
                 nEnd = *counter;
                 while (numBraces > 0)
                 {
                    GET_CONTENTS(s1,s2,nEnd);
                    nEnd += 2;
           
                    GET_FIRST_TOKEN(s2,interpreter_candidate);
              //      printf ("%s : %d\n",interpreter_candidate,numBraces);
                    if (strcmp(interpreter_candidate,"op_brace") == 0)
                       numBraces++;
                    else if (strcmp(interpreter_candidate,"cl_brace") == 0)
                       numBraces--;
                 }
                 
                 *counter = start;
                 while (*counter < nEnd)
                 {
                  //     printf ("Starting to interpret: \n");
                       GET_CONTENTS(s1,s2,*counter);
                       (*counter)+=2;
                       
                       GET_FIRST_TOKEN(s2,interpreter_candidate);
                  //     printf ("%s\n",interpreter_candidate);
                 }
           }
           else
           {
           }
     }
     else
     while (cond_verdict == true)
     {
           GET_CONTENTS(s1,s2,*counter);
           (*counter)+=2;
           
           GET_FIRST_TOKEN(s2,interpreter_candidate);
        //   printf ("%s\n",interpreter_candidate);
           if (strcmp(interpreter_candidate,"op_brace") == 0)
           {
                 int numBraces = 1;
                 nEnd = *counter;
                 while (numBraces > 0)
                 {
                    GET_CONTENTS(s1,s2,nEnd);
                    nEnd += 2;
           
                    GET_FIRST_TOKEN(s2,interpreter_candidate);
              //      printf ("%s : %d\n",interpreter_candidate,numBraces);
                    if (strcmp(interpreter_candidate,"op_brace") == 0)
                       numBraces++;
                    else if (strcmp(interpreter_candidate,"cl_brace") == 0)
                       numBraces--;
                 }
                 
                 *counter = start;
                 while (*counter < nEnd)
                 {
                  //     printf ("Starting to interpret: \n");
                       GET_CONTENTS(s1,s2,*counter);
                       (*counter)+=2;
                       
                       GET_FIRST_TOKEN(s2,interpreter_candidate);
                  //     printf ("%s\n",interpreter_candidate);
                       if (strcmp(interpreter_candidate,"write_optr") == 0)
                       {
                          INTERPRET_WRITE(s1);
                       }
                       else if (strcmp(interpreter_candidate,"read_optr") == 0)
                       {
                          INTERPRET_READ(s1);
                       }
                       else if (strcmp(interpreter_candidate,"int_optr") == 0)
                       {
                          INTERPRET_INT_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"char_optr") == 0)
                       {
                            INTERPRET_CHAR_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"float_optr") == 0)
                       {
                          INTERPRET_FLOAT_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"string_optr") == 0)
                       {
                          INTERPRET_STRING_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
                       {
                           INTERPRET_ASSIGN_STATEMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"if_optr") == 0)
                       {
                            INTERPRET_IF_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"while_optr") == 0)
                       {
                            INTERPRET_WHILE_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"for_optr") == 0)
                       {
                            INTERPRET_FOR_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"do_while_optr") == 0)
                       {
                             INTERPRET_DO_WHILE_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"FUNCTION_CALL") == 0)
                       {
                            CALL_FUNCTION(s1);
                       }
                       else if (strcmp(interpreter_candidate,"set_array") == 0)
                       {
                                 INTERPRET_ARRAY_ASSIGNMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"get_array") == 0)
                       {
                                INTERPRET_ARRAY_GET(s1);
                       }
                                  else if (strcmp(interpreter_candidate,"assign_optr") == 0)
           {
                INTERPRET_ASSIGN_FUNCTION_STATEMENT(s1);
           }
                 }
           }
           else
           {
               if (strcmp(interpreter_candidate,"write_optr") == 0)
               {
                  INTERPRET_WRITE(s1);
               }
               else if (strcmp(interpreter_candidate,"read_optr") == 0)
               {
                  INTERPRET_READ(s1);
               }
               else if (strcmp(interpreter_candidate,"int_optr") == 0)
               {
                  INTERPRET_INT_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"char_optr") == 0)
               {
                    INTERPRET_CHAR_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"float_optr") == 0)
               {
                  INTERPRET_FLOAT_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"string_optr") == 0)
               {
                  INTERPRET_STRING_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
               {
                   INTERPRET_ASSIGN_STATEMENT(s1);
               }
               else if (strcmp(interpreter_candidate,"if_optr") == 0)
               {
                    INTERPRET_IF_STATEMENT(s1,counter);
                    (*counter)--;
               }
           }
           
           INTERPRET_ASSIGN_STATEMENT(update);
            end = false;     
            cond_index = 0;  
            strcpy(empty,"");
            strcpy(value,"");
            while (!end)
            {
                  end2 = false;
                  empty_index = 0;
                  while (!end2)
                  {
                        if (condition[cond_index] != '>' && condition[cond_index] != '<' && condition[cond_index] != '=')
                        {
                           empty[empty_index] = condition[cond_index];
                           empty_index++;
                           cond_index++;
                        }
                        else
                        end2 = true;
                  }
                  empty[empty_index] = '\0';
                  EVALUATE_EQUATION(empty,cond1);
                  empty_index = 0;
                  if (condition[cond_index] == '>' || condition[cond_index] == '<' || condition[cond_index] == '=')
                  {
                            empty[empty_index] = condition[cond_index];
                            cond_index++;
                            empty_index++;
                            if (condition[cond_index] == '=')
                            {
                                          empty[empty_index] = condition[cond_index];
                                          cond_index++;
                                          empty_index++;
                            }   
                            empty[empty_index] = '\0';
                            strcpy(op,empty);
                            if (condition[cond_index] == ' ')
                               cond_index++;  
                       }
                       end2 = false;
                       empty_index = 0;
                       while (!end2)
                       {
                             if (condition[cond_index] != ';')
                             {
                                           empty[empty_index] = condition[cond_index];
                                           empty_index++;
                                           cond_index++;
                             }
                             else
                                 end2 = true;
                       }
                       empty[empty_index] = '\0';
                       EVALUATE_EQUATION(empty,cond2);
                       end = true;
                       
                     //  printf ("%s ? %s\n",cond1,cond2);
                       if (strcmp(op,">") == 0)
                       {
                          if (atoi(cond1) - atoi(cond2) > 0)
                             cond_verdict = true;
                          else
                             cond_verdict = false;
                       }
                       else if (strcmp (op,"<") == 0)
                       {
                          if (atoi(cond1) - atoi(cond2) < 0)
                             cond_verdict = true;
                          else
                             cond_verdict = false;
                       }
                       else if (strcmp (op,">=") == 0)
                       {
                          if (atoi(cond1) - atoi(cond2) >= 0)
                             cond_verdict = true;
                          else
                             cond_verdict = false;
                       }
                       else if (strcmp (op,"<=") == 0)
                       {
                          if (atoi(cond1) - atoi(cond2) <= 0)
                             cond_verdict = true;
                          else
                             cond_verdict = false;
                       }
                       else if (strcmp (op,"==") == 0)
                       {
                          if (atoi(cond1) - atoi(cond2) == 0)
                             cond_verdict = true;
                          else
                             cond_verdict = false;
                       }
                       if (cond_verdict == true)
                          *counter = start;
                       /*
                       if (cond_verdict == true)
                          printf ("TRUE\n");
                       else
                          printf ("FALSE\n");*/
                 }
     }
     
}

void INTERPRET_FOR_STATEMENT_FUNCTION(String Source,int *counter,int index_func,int VARIABLES_START,bool *hasReturned)
{
     String empty;
     String value;
     String cond1;
     String op;
     String cond2;
     String interpreter_candidate;
     String s1,s2;
     String condition;
     String var_loop;
     String var_loop_value;
     
     String update;
     int update_index;
     
     int start = *counter;
     int nEnd;
     int start_condition;
     bool end = false;
     bool end2 = false;
     bool cond_verdict;
     int nSum = 0;
     int cond_index;
     
     strcpy(empty,"");
     strcpy(value,"");
     int i = 0;
     int empty_index = 0;
     while (Source[i] != '(')
           i++;
     i++;
     
     while (Source[i] != ' ' && Source[i] != '=')
     {
           var_loop[empty_index] = Source[i];
           empty_index++;
           i++;
     }
     var_loop[empty_index] = '\0';
     empty_index = 0;
     if (Source[i] == ' ')
        i++;
     if (Source[i] == '=')
        i++;
     if (Source[i] == ' ')
        i++;
     while (Source[i] != ';' && Source[i] != ' ')
     {
           var_loop_value[empty_index] = Source[i];
           empty_index++;
           i++;
     }
     var_loop_value[empty_index] = '\0';
     empty_index = 0;
     UPDATE_VARIABLE(var_loop,var_loop_value);
     if (Source[i] == ' ')
        i++;
     if (Source[i] == ';')
        i++;
     if (Source[i] == ' ')
        i++;
     int temp_i = i;
     while (Source[temp_i] != ' ')
           temp_i++;
     temp_i++;
     while (Source[temp_i] != ';')
     {
           condition[cond_index] = Source[temp_i];
           temp_i++;
           cond_index++;
     }
     condition[cond_index] = ';';
     cond_index++;
     condition[cond_index] = '\0'; 
  //   printf ("Condition detected: %s\n",condition);
     
     update_index = 0;
     
     while (Source[temp_i] != ' ')
           temp_i++;
     temp_i++;
     while (Source[temp_i] != ';')
     {
           update[update_index] = Source[temp_i];
           temp_i++;
           update_index++;
     }
     
     update[update_index] = ';';
     update_index++;
     update[update_index] = '\0';
     
   //  printf ("Assignment equation: %s\n",update);
     cond_index = 0;  
     while (!end)
     {
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (condition[cond_index] != '>' && condition[cond_index] != '<' && condition[cond_index] != '=')
                 {
                               empty[empty_index] = condition[cond_index];
                               empty_index++;
                               cond_index++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond1);
           empty_index = 0;
           if (condition[cond_index] == '>' || condition[cond_index] == '<' || condition[cond_index] == '=')
           {
                empty[empty_index] = condition[cond_index];
                cond_index++;
                empty_index++;
                if (condition[cond_index] == '=')
                {
                              empty[empty_index] = condition[cond_index];
                              cond_index++;
                              empty_index++;
                }   
                empty[empty_index] = '\0';
                strcpy(op,empty);
                if (condition[cond_index] == ' ')
                   cond_index++;  
           }
           end2 = false;
           empty_index = 0;
           while (!end2)
           {
                 if (condition[cond_index] != ';')
                 {
                               empty[empty_index] = condition[cond_index];
                               empty_index++;
                               cond_index++;
                 }
                 else
                     end2 = true;
           }
           empty[empty_index] = '\0';
           EVALUATE_EQUATION(empty,cond2);
           end = true;
           
         //  printf ("%s ? %s\n",cond1,cond2);
           if (strcmp(op,">") == 0)
           {
              if (atoi(cond1) - atoi(cond2) > 0)
                 cond_verdict = true;
              else
                 cond_verdict = false;
           }
           else if (strcmp (op,"<") == 0)
           {
              if (atoi(cond1) - atoi(cond2) < 0)
                 cond_verdict = true;
              else
                 cond_verdict = false;
           }
           else if (strcmp (op,">=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) >= 0)
                 cond_verdict = true;
              else
                 cond_verdict = false;
           }
           else if (strcmp (op,"<=") == 0)
           {
              if (atoi(cond1) - atoi(cond2) <= 0)
                 cond_verdict = true;
              else
                 cond_verdict = false;
           }
           else if (strcmp (op,"==") == 0)
           {
              if (atoi(cond1) - atoi(cond2) == 0)
                 cond_verdict = true;
              else
                 cond_verdict = false;
           }
           
      /*     if (cond_verdict == true)
              printf ("TRUE\n");
           else
              printf ("FALSE\n");*/
     }
     
     if (cond_verdict == false)
     {
            GET_CONTENTS(s1,s2,*counter);
           (*counter)+=2;
           
           GET_FIRST_TOKEN(s2,interpreter_candidate);
           if (strcmp(interpreter_candidate,"op_brace") == 0)
           {
                 int numBraces = 1;
                 nEnd = *counter;
                 while (numBraces > 0)
                 {
                    strcpy(s1,FUNCTIONS[index_func].statements[nEnd]);
                    nEnd++;
                    strcpy(s2,FUNCTIONS[index_func].statements[nEnd]);
                    nEnd++;
           
                    GET_FIRST_TOKEN(s2,interpreter_candidate);
              //      printf ("%s : %d\n",interpreter_candidate,numBraces);
                    if (strcmp(interpreter_candidate,"op_brace") == 0)
                       numBraces++;
                    else if (strcmp(interpreter_candidate,"cl_brace") == 0)
                       numBraces--;
                 }
                 
                 *counter = start;
                 while (*counter < nEnd)
                 {
                       strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
                       (*counter)++;
                       strcpy(s2,FUNCTIONS[index_func].statements[*counter]);
                       (*counter)++;
                  //     printf ("Starting to interpret: \n");

                       GET_FIRST_TOKEN(s2,interpreter_candidate);
                  //     printf ("%s\n",interpreter_candidate);
                 }
           }
           else
           {
           }
     }
     else
     while (cond_verdict == true)
     {
           strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
           (*counter)++;
           strcpy(s2,FUNCTIONS[index_func].statements[*counter]);
           (*counter)++;
           
           GET_FIRST_TOKEN(s2,interpreter_candidate);
        //   printf ("%s\n",interpreter_candidate);
           if (strcmp(interpreter_candidate,"op_brace") == 0)
           {
                 int numBraces = 1;
                 nEnd = *counter;
                 while (numBraces > 0)
                 {
                    strcpy(s1,FUNCTIONS[index_func].statements[nEnd]);
                    nEnd++;
                    strcpy(s2,FUNCTIONS[index_func].statements[nEnd]);
                    nEnd++;
           
                    GET_FIRST_TOKEN(s2,interpreter_candidate);
              //      printf ("%s : %d\n",interpreter_candidate,numBraces);
                    if (strcmp(interpreter_candidate,"op_brace") == 0)
                       numBraces++;
                    else if (strcmp(interpreter_candidate,"cl_brace") == 0)
                       numBraces--;
                 }
                 
                 *counter = start;
                 while (*counter < nEnd && *hasReturned == false)
                 {
                  //     printf ("Starting to interpret: \n");
                         strcpy(s1,FUNCTIONS[index_func].statements[*counter]);
                         (*counter)++;
                         strcpy(s2,FUNCTIONS[index_func].statements[*counter]);
                         (*counter)++;
                       
                       GET_FIRST_TOKEN(s2,interpreter_candidate);
                  //     printf ("%s\n",interpreter_candidate);
                       if (strcmp(interpreter_candidate,"write_optr") == 0)
                       {
                          INTERPRET_WRITE(s1);
                       }
                       else if (strcmp(interpreter_candidate,"read_optr") == 0)
                       {
                          INTERPRET_READ(s1);
                       }
                       else if (strcmp(interpreter_candidate,"int_optr") == 0)
                       {
                          INTERPRET_INT_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"char_optr") == 0)
                       {
                            INTERPRET_CHAR_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"float_optr") == 0)
                       {
                          INTERPRET_FLOAT_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"string_optr") == 0)
                       {
                          INTERPRET_STRING_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
                       {
                           INTERPRET_ASSIGN_STATEMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"if_optr") == 0)
                       {
                            INTERPRET_IF_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                       }
                       else if (strcmp(interpreter_candidate,"while_optr") == 0)
                       {
                            INTERPRET_WHILE_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                       }
                       else if (strcmp(interpreter_candidate,"for_optr") == 0)
                       {
                            INTERPRET_FOR_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                       }
                       else if (strcmp(interpreter_candidate,"do_while_optr") == 0)
                       {
                            INTERPRET_DO_WHILE_STATEMENT_FUNCTION(s1,counter,index_func,VARIABLES_START,hasReturned);
                       }
                       else if (strcmp(interpreter_candidate,"FUNCTION_CALL") == 0)
                       {
                            CALL_FUNCTION(s1);
                       }
                       else if (strcmp(interpreter_candidate,"return_optr") == 0)
                       {
                            INTERPRET_RETURN_STATEMENT(s1,index_func,VARIABLES_START);
                            *counter = nEnd;
                            *hasReturned = true;
                                return;
                       }
                       else if (strcmp(interpreter_candidate,"set_array") == 0)
                       {
                                 INTERPRET_ARRAY_ASSIGNMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"get_array") == 0)
                       {
                                INTERPRET_ARRAY_GET(s1);
                       }
                                  else if (strcmp(interpreter_candidate,"assign_optr") == 0)
           {
                INTERPRET_ASSIGN_FUNCTION_STATEMENT(s1);
           }
                 }
           }
           else
           {
               if (strcmp(interpreter_candidate,"write_optr") == 0)
               {
                  INTERPRET_WRITE(s1);
               }
               else if (strcmp(interpreter_candidate,"read_optr") == 0)
               {
                  INTERPRET_READ(s1);
               }
               else if (strcmp(interpreter_candidate,"int_optr") == 0)
               {
                  INTERPRET_INT_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"char_optr") == 0)
               {
                    INTERPRET_CHAR_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"float_optr") == 0)
               {
                  INTERPRET_FLOAT_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"string_optr") == 0)
               {
                  INTERPRET_STRING_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
               {
                   INTERPRET_ASSIGN_STATEMENT(s1);
               }
               else if (strcmp(interpreter_candidate,"if_optr") == 0)
               {
                    INTERPRET_IF_STATEMENT(s1,counter);
                    (*counter)--;
               }
           }
           
           INTERPRET_ASSIGN_STATEMENT(update);
            end = false;     
            cond_index = 0;  
            strcpy(empty,"");
            strcpy(value,"");
            while (!end)
            {
                  end2 = false;
                  empty_index = 0;
                  while (!end2)
                  {
                        if (condition[cond_index] != '>' && condition[cond_index] != '<' && condition[cond_index] != '=')
                        {
                           empty[empty_index] = condition[cond_index];
                           empty_index++;
                           cond_index++;
                        }
                        else
                        end2 = true;
                  }
                  empty[empty_index] = '\0';
                  EVALUATE_EQUATION(empty,cond1);
                  empty_index = 0;
                  if (condition[cond_index] == '>' || condition[cond_index] == '<' || condition[cond_index] == '=')
                  {
                            empty[empty_index] = condition[cond_index];
                            cond_index++;
                            empty_index++;
                            if (condition[cond_index] == '=')
                            {
                                          empty[empty_index] = condition[cond_index];
                                          cond_index++;
                                          empty_index++;
                            }   
                            empty[empty_index] = '\0';
                            strcpy(op,empty);
                            if (condition[cond_index] == ' ')
                               cond_index++;  
                       }
                       end2 = false;
                       empty_index = 0;
                       while (!end2)
                       {
                             if (condition[cond_index] != ';')
                             {
                                           empty[empty_index] = condition[cond_index];
                                           empty_index++;
                                           cond_index++;
                             }
                             else
                                 end2 = true;
                       }
                       empty[empty_index] = '\0';
                       EVALUATE_EQUATION(empty,cond2);
                       end = true;
                       
                     //  printf ("%s ? %s\n",cond1,cond2);
                       if (strcmp(op,">") == 0)
                       {
                          if (atoi(cond1) - atoi(cond2) > 0)
                             cond_verdict = true;
                          else
                             cond_verdict = false;
                       }
                       else if (strcmp (op,"<") == 0)
                       {
                          if (atoi(cond1) - atoi(cond2) < 0)
                             cond_verdict = true;
                          else
                             cond_verdict = false;
                       }
                       else if (strcmp (op,">=") == 0)
                       {
                          if (atoi(cond1) - atoi(cond2) >= 0)
                             cond_verdict = true;
                          else
                             cond_verdict = false;
                       }
                       else if (strcmp (op,"<=") == 0)
                       {
                          if (atoi(cond1) - atoi(cond2) <= 0)
                             cond_verdict = true;
                          else
                             cond_verdict = false;
                       }
                       else if (strcmp (op,"==") == 0)
                       {
                          if (atoi(cond1) - atoi(cond2) == 0)
                             cond_verdict = true;
                          else
                             cond_verdict = false;
                       }
                       if (cond_verdict == true)
                          *counter = start;
                       /*
                       if (cond_verdict == true)
                          printf ("TRUE\n");
                       else
                          printf ("FALSE\n");*/
                 }
     }
     
}

int count_Parameters(String CAND)
{
    int i = 0,v = strlen(CAND);
    int numParameters = 0;
    while (CAND[i] != '(')
          i++;
    i++;
    if (CAND[i] == ')')
       return 0;
    else
    {
        numParameters++;
        while (CAND[i] != ')')
        {
              if (CAND[i] == ',')
                 numParameters++;
              i++;
        }
    }
    return numParameters;
}

void Strip(String str)
{
     int i,v = strlen(str);
     for (i = 0;i < v;i++)
     {
         if (str[i] == '(')
         {
                    str[i] = '\0';
                    break;
         }
     }
}

void INTERPRET_RETURN_STATEMENT(String CAND,int index_func,int VARIABLES_START)
{
     int i = 0,v = strlen(CAND);
     String temp_string;
     String new_value;
     int temp_string_index = 0;
     while (CAND[i] != ' ')
           i++;
     i++;
     while (CAND[i] != ';')
     {
           temp_string[temp_string_index] = CAND[i];
           i++;
           temp_string_index++;
     }
     temp_string[temp_string_index] = '\0';
 //    printf ("%s\n",temp_string);
     EVALUATE_EQUATION(temp_string,new_value);
     strcpy(FUNCTIONS[index_func].return_value,new_value);    
     REMOVE_LOCAL_VARIABLES(VARIABLES_START);
}

void CALL_FUNCTION(String CAND)
{
     String temp_string;
     strcpy(temp_string,CAND);
     Strip(temp_string);
     int i = ADD_FUNCTION(temp_string),count = 0,h;
     bool hasReturned = false;
     
     if (WATCH_AND_TRACE)
     {
        printf ("Called function: %s\n",temp_string);
        getch();
     }
     String s1,s2,interpreter_candidate;
     
     int VARIABLES_START = VARIABLES_INDEX;
     int numParameters = count_Parameters(CAND);
     int z = 0;
     
     String ts1,output;
     int ts_index;
     while (CAND[z] != '(')
           z++;
     z++;
     h = 0;
     while (h < FUNCTIONS[i].num_parameters)
     {
           ts_index = 0;
           while (CAND[z] != ',' && CAND[z] != ')')
           {
                 ts1[ts_index] = CAND[z];
                 z++;
                 ts_index++;
           }
           ts1[ts_index] = '\0';
           EVALUATE_EQUATION(ts1,output);
           NEW_VARIABLE(FUNCTIONS[i].parameter_types[h],FUNCTIONS[i].parameters[h],output);
           h++;
           z++;
     }
    // printf ("Number of parameters for function call: %d\n",numParameters);
    // if (numParameters != FUNCTIONS[i].num_parameters)
    // {
   //                    printf ("ERROR! Mismatching parameters exception: \n");
    // }
     count = 0;
     while (count < FUNCTIONS[i].num_statements && !hasReturned)
     {
           
           strcpy(s1,FUNCTIONS[i].statements[count]);
           count++;
           strcpy(s2,FUNCTIONS[i].statements[count]);
           count++;
           GET_FIRST_TOKEN(s2,interpreter_candidate);
           if (strcmp(interpreter_candidate,"write_optr") == 0)
           {
              INTERPRET_WRITE(s1);
           }
           else if (strcmp(interpreter_candidate,"read_optr") == 0)
           {
              INTERPRET_READ(s1);
           }
           else if (strcmp(interpreter_candidate,"int_optr") == 0)
           {
              //  printf ("It freezes here\n");
              INTERPRET_INT_IDENTIFY(s1);
           }
           else if (strcmp(interpreter_candidate,"char_optr") == 0)
           {
                INTERPRET_CHAR_IDENTIFY(s1);
           }
           else if (strcmp(interpreter_candidate,"float_optr") == 0)
           {
              INTERPRET_FLOAT_IDENTIFY(s1);
           }
           else if (strcmp(interpreter_candidate,"string_optr") == 0)
           {
              INTERPRET_STRING_IDENTIFY(s1);
           }
           else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
           {
               INTERPRET_ASSIGN_STATEMENT(s1);
           }
           else if (strcmp(interpreter_candidate,"if_optr") == 0)
           {
               // printf ("IF INSIDE A FUNCTION?\n");
                INTERPRET_IF_STATEMENT_FUNCTION(s1,&count,i,VARIABLES_START,&hasReturned);
           }
           else if (strcmp(interpreter_candidate,"while_optr") == 0)
           {
                INTERPRET_WHILE_STATEMENT_FUNCTION(s1,&count,i,VARIABLES_START,&hasReturned);
           }
           else if (strcmp(interpreter_candidate,"for_optr") == 0)
           {
                INTERPRET_FOR_STATEMENT_FUNCTION(s1,&count,i,VARIABLES_START,&hasReturned);
           }
           else if (strcmp(interpreter_candidate,"do_while_optr") == 0)
           {
                INTERPRET_DO_WHILE_STATEMENT_FUNCTION(s1,&count,i,VARIABLES_START,&hasReturned);
           }
           else if (strcmp(interpreter_candidate,"FUNCTION_CALL") == 0)
           {
                CALL_FUNCTION(s1);
           }
           else if (strcmp(interpreter_candidate,"return_optr") == 0)
           {
                INTERPRET_RETURN_STATEMENT(s1,i,VARIABLES_START);
                if (IMPROVED_WATCH_AND_TRACE)
                {
                     printf ("<BREAKPOINT: Exited function %s returning %s>\n",FUNCTIONS[i].name,FUNCTIONS[i].return_value);
                     WRITE_VARIABLE_DATA();
                     getch();
                }
                return;
            //    printf ("Returned value: %s\n",FUNCTIONS[i].return_value);
           }
           else if (strcmp(interpreter_candidate,"assign_optr") == 0)
           {
                INTERPRET_ASSIGN_FUNCTION_STATEMENT(s1);
           }
     }
     REMOVE_LOCAL_VARIABLES(VARIABLES_START);
     if (IMPROVED_WATCH_AND_TRACE)
     {
        printf ("<BREAKPOINT: Exited function %s>\n",FUNCTIONS[i].name);
        WRITE_VARIABLE_DATA();
        getch();
     }
}

void INTERPRET_CATCH_STATEMENT(String Source,int *counter)
{
     String empty;
     String value;
     String cond1;
     String op;
     String cond2;
     String interpreter_candidate;
     String s1,s2;
     
     int start = *counter;
     int nEnd;
     
     bool end = false;
     bool end2 = false;
     bool cond_verdict;
     int nSum = 0;
     
     int finVerdict = 0;
     int conVerdict = 0;
     
     bool bor = false;
     bool band = false;
     strcpy(empty,"");
     strcpy(value,"");
     int i = 0;
     int empty_index = 0;

     //if (RUNTIME_ERROR == true)
     //    printf ("RUNTIME ERROR detected!\n");
     //else
     //    printf ("IGNORING\n");
     if (RUNTIME_ERROR == false)
     {
            GET_CONTENTS(s1,s2,*counter);
           (*counter)+=2;
           
           GET_FIRST_TOKEN(s2,interpreter_candidate);
           if (strcmp(interpreter_candidate,"op_brace") == 0)
           {
                 int numBraces = 1;
                 nEnd = *counter;
                 while (numBraces > 0)
                 {
                    GET_CONTENTS(s1,s2,nEnd);
                    nEnd += 2;
           
                    GET_FIRST_TOKEN(s2,interpreter_candidate);
                    if (strcmp(interpreter_candidate,"op_brace") == 0)
                       numBraces++;
                    else if (strcmp(interpreter_candidate,"cl_brace") == 0)
                       numBraces--;
                 }
                 
                 *counter = start;
                 while (*counter < nEnd)
                 {
                       GET_CONTENTS(s1,s2,*counter);
                       (*counter)+=2;
                       
                       GET_FIRST_TOKEN(s2,interpreter_candidate);
                 }
           }
           else
           {
           }
     }
     else
     {
           RUNTIME_ERROR = false;
           GET_CONTENTS(s1,s2,*counter);
           (*counter)+=2;
           
           GET_FIRST_TOKEN(s2,interpreter_candidate);
           if (strcmp(interpreter_candidate,"op_brace") == 0)
           {
                 int numBraces = 1;
                 nEnd = *counter;
                 while (numBraces > 0)
                 {
                    GET_CONTENTS(s1,s2,nEnd);
                    nEnd += 2;
           
                    GET_FIRST_TOKEN(s2,interpreter_candidate);
                    if (strcmp(interpreter_candidate,"op_brace") == 0)
                       numBraces++;
                    else if (strcmp(interpreter_candidate,"cl_brace") == 0)
                       numBraces--;
                 }
                 
                 *counter = start;
                 while (*counter < nEnd)
                 {
                       GET_CONTENTS(s1,s2,*counter);
                       (*counter)+=2;
                       
                       GET_FIRST_TOKEN(s2,interpreter_candidate);
                       if (strcmp(interpreter_candidate,"write_optr") == 0)
                       {
                      //    if (!RUNTIME_ERROR)
                             INTERPRET_WRITE(s1);
                       }
                       else if (strcmp(interpreter_candidate,"read_optr") == 0)
                       {
                       //      if (!RUNTIME_ERROR)
                          INTERPRET_READ(s1);
                       }
                       else if (strcmp(interpreter_candidate,"int_optr") == 0)
                       {
                        //     if (!RUNTIME_ERROR)
                          INTERPRET_INT_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"char_optr") == 0)
                       {
                      //       if (!RUNTIME_ERROR)
                            INTERPRET_CHAR_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"float_optr") == 0)
                       {
                         //    if (!RUNTIME_ERROR)
                          INTERPRET_FLOAT_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"string_optr") == 0)
                       {
                        //     if (!RUNTIME_ERROR)
                          INTERPRET_STRING_IDENTIFY(s1);
                       }
                       else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
                       {
                         //    if (!RUNTIME_ERROR)
                           INTERPRET_ASSIGN_STATEMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"if_optr") == 0)
                       {
                         //    if (!RUNTIME_ERROR)
                            INTERPRET_IF_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"while_optr") == 0)
                       {
                         //    if (!RUNTIME_ERROR)
                            INTERPRET_WHILE_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"do_while_optr") == 0)
                       {
                         //    if (!RUNTIME_ERROR)
                                INTERPRET_DO_WHILE_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"for_optr") == 0)
                       {
                           //  if (!RUNTIME_ERROR)
                                INTERPRET_FOR_STATEMENT(s1,counter);
                       }
                       else if (strcmp(interpreter_candidate,"FUNCTION_CALL") == 0)
                       {
                          //   if (!RUNTIME_ERROR)
                            CALL_FUNCTION(s1);
                       }
                       else if (strcmp(interpreter_candidate,"set_array") == 0)
                       {
                          //   if (!RUNTIME_ERROR)
                                 INTERPRET_ARRAY_ASSIGNMENT(s1);
                       }
                       else if (strcmp(interpreter_candidate,"get_array") == 0)
                       {
                         //    if (!RUNTIME_ERROR)
                                INTERPRET_ARRAY_GET(s1);
                       }
                       else if (strcmp(interpreter_candidate,"assign_optr") == 0)
                       {
                          //   if (!RUNTIME_ERROR)
                                  INTERPRET_ASSIGN_FUNCTION_STATEMENT(s1);
                       }
                 }
           }
           else
           {
               if (strcmp(interpreter_candidate,"write_optr") == 0)
               {
                  INTERPRET_WRITE(s1);
               }
               else if (strcmp(interpreter_candidate,"read_optr") == 0)
               {
                  INTERPRET_READ(s1);
               }
               else if (strcmp(interpreter_candidate,"int_optr") == 0)
               {
                  INTERPRET_INT_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"char_optr") == 0)
               {
                    INTERPRET_CHAR_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"float_optr") == 0)
               {
                  INTERPRET_FLOAT_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"string_optr") == 0)
               {
                  INTERPRET_STRING_IDENTIFY(s1);
               }
               else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
               {
                   INTERPRET_ASSIGN_STATEMENT(s1);
               }
               else if (strcmp(interpreter_candidate,"if_optr") == 0)
               {
                    INTERPRET_IF_STATEMENT(s1,counter);
                    (*counter)--;
               }
           }
     
     }
}     

void INTERPRET_ASSIGN_FUNCTION_STATEMENT (String CAND)
{
     String temp_string;
     String variable,new_string;
     int temp_string_index = 0;
     int i = 0,func_ind;
     while (CAND[i] != '(')
           i++;
     i++;
     while (CAND[i] != ')')
     {
           temp_string[temp_string_index] = CAND[i];
           temp_string_index++;
           i++;
     }
     temp_string[temp_string_index] = '\0';
     strcpy(variable,temp_string);
     i++;
     if (CAND[i] == ' ')
        i++;
     if (CAND[i] == '=')
        i++;
     if (CAND[i] == ' ')
        i++;
     temp_string_index = 0;
     while (CAND[i] != ';')
     {
           temp_string[temp_string_index] = CAND[i];
           i++;
           temp_string_index++;
     }
     temp_string[temp_string_index] = '\0';
     strcpy(new_string,temp_string);
    // printf ("Calling function: %s\n",new_string);
     CALL_FUNCTION(temp_string);
     Strip(new_string);
     func_ind = ADD_FUNCTION(new_string);
   // printf ("Updating variable: %s\n",variable);
  //   printf ("Function name: %s\n",FUNCTIONS[func_ind].name);
  //   printf ("Obtained value: %s\n",FUNCTIONS[func_ind].return_value);
     UPDATE_VARIABLE(variable,FUNCTIONS[func_ind].return_value);
}

void INTERPRET_CONSTANT_DECLARATION(String Cand)
{
     int i = 0,v = strlen(Cand);
   //  printf ("%s\n",Cand);
     String temp_string;
     String constant_name;
     int temp_string_index = 0;
     i++;
     while (Cand[i] != ' ')
     {
           temp_string[temp_string_index] = Cand[i];
           i++;
           temp_string_index++;
     }
     i++;
     temp_string[temp_string_index] = '\0';
     strcpy(constant_name,temp_string);
     temp_string_index = 0;
     while (Cand[i] != ' ')
           i++;
     i++;
     while (Cand[i] != ';')
     {
           temp_string[temp_string_index] = Cand[i];
           i++;
           temp_string_index++;
     }
     temp_string[temp_string_index] = '\0';
     NEW_CONSTANT(constant_name,temp_string);
   //  printf ("New constant: %s %s\n",constant_name,temp_string);
   //  getch();
}

int GET_NAME_TYPE(String cand)
{
    if (strcmp(cand,"int") == 0)
       return INT;
    else if (strcmp(cand,"float") == 0)
       return FLOAT;
    else if (strcmp(cand,"char") == 0)
       return CHAR;
    else if (strcmp(cand,"string") == 0)
       return STRING;
    else if (strcmp(cand,"boolean") == 0)
       return BOOLEAN;
    else
       return -1;
}

void INTERPRET_ARRAY_GET(String cand)
{
     int i = 0,v = strlen(cand);
     String temp_string;
     String array_name;
     String output;
     String value;
     String var_name;
     int temp_string_index;
     int array_index;
     
     while (cand[i] != '(')
           i++;
     i++;
     temp_string_index = 0;
     while (cand[i] != '[')
     {
           temp_string[temp_string_index] = cand[i];
           i++;
           temp_string_index++;
     }
     temp_string[temp_string_index] = '\0';
     strcpy(array_name,temp_string);
     i++;
     temp_string_index = 0;
     while (cand[i] != ']')
     {
           temp_string[temp_string_index] = cand[i];
           i++;
           temp_string_index++;
     }
     temp_string[temp_string_index] = '\0';
     EVALUATE_EQUATION(temp_string,output);
     array_index = atoi(output);
     i++;
     if (cand[i] == ',')
        i++;
     if (cand[i] == ' ')
        i++;
     temp_string_index = 0;
     while (cand[i] != ')')
     {
           temp_string[temp_string_index] = cand[i];
           i++;
           temp_string_index++;
     }
     temp_string[temp_string_index] = '\0';
     strcpy(var_name,temp_string);
     GET_ARRAY(array_name,array_index,value);
     if (WATCH_AND_TRACE)
     {
        printf ("<Obtaining index %d from %s and assigning it to %s>\n",array_index,array_name,value);
        getch();
     }
     UPDATE_VARIABLE(var_name,value);
}

void INTERPRET_ARRAY_ASSIGNMENT(String cand)
{
     int i = 0,v = strlen(cand);
     String temp_string;
     String array_name;
     String output;
     String value;
     String var_name;
     
     int temp_string_index;
     int array_index;
     
     while (cand[i] != '(')
           i++;
     i++;
     temp_string_index = 0;
     while (cand[i] != '[')
     {
           temp_string[temp_string_index] = cand[i];
           i++;
           temp_string_index++;
     }
     temp_string[temp_string_index] = '\0';
     strcpy(array_name,temp_string);
     i++;
     temp_string_index = 0;
     while (cand[i] != ']')
     {
           temp_string[temp_string_index] = cand[i];
           i++;
           temp_string_index++;
     }
     temp_string[temp_string_index] = '\0';
     EVALUATE_EQUATION(temp_string,output);
     array_index = atoi(output);
     i++;
     if (cand[i] == ',')
        i++;
     if (cand[i] == ' ')
        i++;
     temp_string_index = 0;
     while (cand[i] != ')')
     {
           temp_string[temp_string_index] = cand[i];
           i++;
           temp_string_index++;
     }
     temp_string[temp_string_index] = '\0';
     EVALUATE_EQUATION(temp_string,value);
     if (WATCH_AND_TRACE)
     {
        printf ("<Setting value at index %d from %s with %s>\n",array_index,array_name,value);
        getch();
     }
   //  strcpy(value,temp_string);
     UPDATE_ARRAY(array_name,array_index,value);
}

void INTERPRET_ARRAY_DECLARATION(String cand)
{
     int i = 0,v = strlen(cand);
     
     String temp_string;
     String array_name;
     
     int array_size;
     int temp_string_index;
     int array_type;
     while (cand[i] != ' ')
           i++;
     i++;
     temp_string_index = 0;
     while (cand[i] != ' ')
     {
           temp_string[temp_string_index] = cand[i];
           i++;
           temp_string_index++;
     }
     temp_string[temp_string_index] = '\0';
     array_type = GET_NAME_TYPE(temp_string);
     i++;
     temp_string_index = 0;
     while (cand[i] != '[')
     {
           temp_string[temp_string_index] = cand[i];
           i++;
           temp_string_index++;
     }
     temp_string[temp_string_index] = '\0';
     strcpy(array_name,temp_string);
     i++;
     temp_string_index = 0;
     while (cand[i] != ']')
     {
           temp_string[temp_string_index] = cand[i];
           i++;
           temp_string_index++;
     }
     temp_string[temp_string_index] = '\0';
     array_size = atoi(temp_string);
     NEW_ARRAY(array_type,array_name,array_size);
}

void main_interpreter()
{
     String s1,s2,interpreter_candidate;
     CLEAR_STACK();
     FILE *pFile = fopen("Syntax.art","rt");
   //  printf ("Interpreting...\n");    
     int counter = 0;
     while (fgets(STR_NEXT_LINE, 1000, pFile) != NULL)
     {
           INSERT_TO_STACK_2(STR_NEXT_LINE);
     }
     fclose(pFile);
     
     do
     {
         GET_CONTENTS(s1,s2,counter);
         GET_FIRST_TOKEN(s2,interpreter_candidate);
         if (strcmp(interpreter_candidate,"number_sign") == 0)
         {
             INTERPRET_CONSTANT_DECLARATION(s1);
         }
         else if (strcmp(interpreter_candidate,"array_optr") == 0)
         {
             INTERPRET_ARRAY_DECLARATION(s1);
         }
         counter+=2;
     } while (strcmp(interpreter_candidate,"main_optr") != 0);
     
     while (counter < STACK_2_INDEX)
     {
           
           GET_CONTENTS(s1,s2,counter);
           counter+=2;
           
           GET_FIRST_TOKEN(s2,interpreter_candidate);
           if (strcmp(interpreter_candidate,"write_optr") == 0)
           {
           if (!RUNTIME_ERROR)
              INTERPRET_WRITE(s1);
           }
           else if (strcmp(interpreter_candidate,"read_optr") == 0)
           {
           if (!RUNTIME_ERROR)
              INTERPRET_READ(s1);
           }
           else if (strcmp(interpreter_candidate,"int_optr") == 0)
           {
            if (!RUNTIME_ERROR)
              INTERPRET_INT_IDENTIFY(s1);
           }
           else if (strcmp(interpreter_candidate,"char_optr") == 0)
           {
             if (!RUNTIME_ERROR)
                INTERPRET_CHAR_IDENTIFY(s1);
           }
           else if (strcmp(interpreter_candidate,"float_optr") == 0)
           {
            if (!RUNTIME_ERROR)
              INTERPRET_FLOAT_IDENTIFY(s1);
           }
           else if (strcmp(interpreter_candidate,"string_optr") == 0)
           {
            if (!RUNTIME_ERROR)
              INTERPRET_STRING_IDENTIFY(s1);
           }
           else if (strcmp(interpreter_candidate,"IDENTIFIER") == 0)
           {
            if (!RUNTIME_ERROR)
               INTERPRET_ASSIGN_STATEMENT(s1);
           }
           else if (strcmp(interpreter_candidate,"if_optr") == 0)
           {
            if (!RUNTIME_ERROR)
                INTERPRET_IF_STATEMENT(s1,&counter);
           }
           else if (strcmp(interpreter_candidate,"while_optr") == 0)
           {
            if (!RUNTIME_ERROR)
                INTERPRET_WHILE_STATEMENT(s1,&counter);
           }
           else if (strcmp(interpreter_candidate,"FUNCTION_CALL") == 0)
           {
            if (!RUNTIME_ERROR)
                CALL_FUNCTION(s1);
           }
           else if (strcmp(interpreter_candidate,"for_optr") == 0)
           {
            if (!RUNTIME_ERROR)
                INTERPRET_FOR_STATEMENT(s1,&counter);
           }
           else if (strcmp(interpreter_candidate,"do_while_optr") == 0)
           {
            if (!RUNTIME_ERROR)
                INTERPRET_DO_WHILE_STATEMENT(s1,&counter);
           }
           else if (strcmp(interpreter_candidate,"assign_optr") == 0)
           {
            if (!RUNTIME_ERROR)
                INTERPRET_ASSIGN_FUNCTION_STATEMENT(s1);
           }
           else if (strcmp(interpreter_candidate,"set_array") == 0)
           {
            if (!RUNTIME_ERROR)
                INTERPRET_ARRAY_ASSIGNMENT(s1);
           }
           else if (strcmp(interpreter_candidate,"get_array") == 0)
           {
            if (!RUNTIME_ERROR)
                INTERPRET_ARRAY_GET(s1);
           }
           else if (strcmp(interpreter_candidate,"catch_optr") == 0)
           {
                INTERPRET_CATCH_STATEMENT(s1,&counter);
           }
     }
     if (RUNTIME_ERROR)
     {
        printf ("\n");
        printf ("UNCAUGHT EXCEPTION REPORTED!\n");
        switch (EXCEPTION_TYPE)
        {
               case NUMBER_FORMAT_EXCEPTION:
                    printf ("\tNUMBER_FORMAT_EXCEPTION reported\n");
                    printf ("\tAttempted to load invalid number into a numerical data type\n");
                    printf ("\tExit signal obtained\n");
                    break;
               case VARIABLE_MISMATCH_EXCEPTION:
                    printf ("\tVARIABLE_MISMATCH_EXCEPTION reported\n");
                    printf ("\tAttempted convert into data but failed during conversion type\n");
                    printf ("\tExit signal obtained\n");
                    break;
               case ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION:
                    printf ("\tARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION reported\n");
                    printf ("\tAccessed invalid section in the memory segment\n");
                    printf ("\tPointer encountered invalid section before reaching desired section\n");
                    printf ("\tExit signal obtained\n");
                    break;
               case DIVISION_BY_ZERO_EXCEPTION:
                    printf ("\tDIVISION_BY_ZERO_EXCEPTION reported\n");
                    printf ("\tUnable to perform complete arithmetic operation\n");
                    printf ("\tInvalid operation encountered. Division by '0' cannot be completed\n");
                    printf ("\tExit signal obtained\n");
                    break;
        }
     }
     printf ("\n");
     printf ("INTERPRETER HAS FINISHED INTERPRETING\n");
     printf ("press a key to continue");
}

/******************************** FUNCTIONS SUPPORTED *****************************************/

int countParameters(String cand)
{
    int i,v = strlen(cand);
    int numParameters = 0;
    i = 0;
    while (cand[i] != '(')
          i++;
    i++;
    if (cand[i] == ' ')
       i++;
    if (cand[i] == ')')
       return 0;
    else
    {
        numParameters++;
        while (cand[i] != ')')
        {
              if (cand[i] == ',')
                 numParameters++;
              i++;
        }
    }
    return numParameters;
}

void Dissect_Function_Name(String cand,String target)
{
	int i,v = strlen(cand);
	int target_index = 0;
	strcpy(target,"");
	i = 0;
	while (cand[i] != ' ')
	{
		i++;
    }
	i++;
	while (cand[i] != ' ')
	{
        i++;
	//	printf ("Current character: %c\n",cand[i]);
    }
    i++;
    //printf ("Current character: %c\n",cand[i]);
	while (cand[i] != ' ' && cand[i] != '(')
	{
     //   printf ("%c",cand[i]);
		target[target_index] = cand[i];
		i++;
		target_index++;
	}
//	printf ("\n");
	target[target_index] = '\0';
//	printf ("Function name: \n",target);
}

/******************************** INTERPRETER END *********************************************/

void populate_parameters(int func_index,String CAND)
{
     int i = 0,v = strlen(CAND);
     String temp_string;
     int temp_string_index;
     int param_index = 0;
     while (CAND[i] != '(')
                 i++;
     while (i < v && CAND[i] != ')')
     {
           i++;
           temp_string_index = 0;
           while (CAND[i] != ' ')
           {
                 temp_string[temp_string_index] = CAND[i];
                 i++;
                 temp_string_index++;
           }
           temp_string[temp_string_index] = '\0';
           if (strcmp(temp_string,"int"))
              FUNCTIONS[func_index].parameter_types[param_index] = INT;
           else if (strcmp(temp_string,"char"))
              FUNCTIONS[func_index].parameter_types[param_index] = CHAR;
           else if (strcmp(temp_string,"float"))
              FUNCTIONS[func_index].parameter_types[param_index] = FLOAT;   
           else if (strcmp(temp_string,"string"))
              FUNCTIONS[func_index].parameter_types[param_index] = STRING;
           else if (strcmp(temp_string,"boolean"))
              FUNCTIONS[func_index].parameter_types[param_index] = BOOLEAN;
           i++;
           temp_string_index = 0;
           while (CAND[i] != ',' && CAND[i] != ')')
           {
                 temp_string[temp_string_index] = CAND[i];
                 i++;
                 temp_string_index++;
           } 
           temp_string[temp_string_index] = '\0';
           strcpy(FUNCTIONS[func_index].parameters[param_index],temp_string);  
           param_index++;
           i++;                      
     }
}

void populate_to_functions()
{
	String statements[10000];
	
	String s1,s2,interpreter_candidate;
    CLEAR_STACK();
    FILE *pFile = fopen("Syntax.art","rt");
     
    int counter = 0;
    while (fgets(STR_NEXT_LINE, 1000, pFile) != NULL)
    {
           strcpy(statements[counter],STR_NEXT_LINE);
		   counter++;
    }
    fclose(pFile);
}

bool Evaluate_Call(String cand)
{
     String temp_string;
     strcpy(temp_string,cand);
     Strip(temp_string);
     
     int i = ADD_FUNCTION(temp_string);
     
     int num_parameters1 = count_Parameters(cand);
     
   //  printf ("%s: %d ?= %d\n",FUNCTIONS[i].name,FUNCTIONS[i].num_parameters,num_parameters1);
     //getch();
     if (FUNCTIONS[i].num_parameters != num_parameters1)
        return false;
     else
       return true;
}

bool Evaluate_Assign(String cand)
{
     int i = 0,temp_i;
     int var_type,func_type;
     String temp_string;
     String func_name;
     int temp_string_index;
     while (cand[i] != '(')
           i++;
     i++;
     temp_string_index = 0;
     while (cand[i] != ')')
     {
           temp_string[temp_string_index] = cand[i];
           i++;
           temp_string_index++;
     }
     temp_string[temp_string_index] = '\0';
     
     var_type = getVariableType(temp_string);
     i++;
     if (cand[i] == ' ')
        i++;
     if (cand[i] == '=')
        i++;
     if (cand[i] == ' ')
        i++;
     temp_string_index = 0;
     temp_i = i;
     while (cand[temp_i] != '(')
     {
           temp_string[temp_string_index] = cand[temp_i];
           temp_i++;
           temp_string_index++;
     }
     temp_string[temp_string_index] = '\0';
     func_type = GET_FUNCTION_TYPE(temp_string);
    // printf ("%d ?= %d\n",func_type,var_type);
    // getch();
     if (func_type != var_type)
        return false;
     else
     {
         temp_string_index = 0;
         while (cand[i] != ';')
         {
               temp_string[temp_string_index] = cand[i];
               i++;
               temp_string_index++;
         }
         temp_string[temp_string_index] = '\0';
         if (!Evaluate_Call(temp_string))
            return false;
         else
            return true;
     }
     return true;
}

int determine(String cand)
{
    if (strcmp(cand,"int") == 0)
       return INT;
    else if (strcmp(cand,"void") == 0)
       return -1;
    else if (strcmp(cand,"float") == 0)
       return FLOAT;
    else if (strcmp(cand,"char") == 0)
       return CHAR;
    else if (strcmp(cand,"boolean") == 0)
       return BOOLEAN;
    else if (strcmp(cand,"string") == 0)
       return STRING;
}

int getFunctionType(String cand)
{
    int i = 0,v = strlen(cand);
    String temp_string;
    int temp_string_index;
    while (cand[i] != ' ')
          i++;
    i++;
    temp_string_index = 0;
    while (cand[i] != ' ')
    {
          temp_string[temp_string_index] = cand[i];
          i++;
          temp_string_index++;
    }
    temp_string[temp_string_index] = '\0';
    return determine(temp_string);
}

int main(int argc,char *argv[])
{
    int n = 0;
//	String statements[10000];
    /* Redirect Standard input and output into the desired files*/
    #ifdef TAKE_INPUT_FROM_TEXT_FILE
   // freopen(INPUT_FILENAME,"rt",stdin);
    FILE *file;
    file = fopen(INPUT_FILENAME,"rt");
    String s1,s2;
    #endif
     
    #ifdef WRITE_OUTPUT_TO_TEXT_FILE
 //   freopen(OUTPUT_FILENAME,"wt",stdout);
    pFile = fopen(OUTPUT_FILENAME,"wt");
    pErrorFile = fopen(ERROR_LOG,"wt");
    #endif
    /* End redirecting here */
    
    /* Initialize the main contents */
    #ifdef SAFE_RELEASE
    INIT_MAIN_PROGRAM_CONTENTS();
    #endif
    
    /* End the initialization of main contents */
    
    String func_token;	
   /* Initial main loop: Continously get an input until the end of the input file */
   
   fclose(file);
   
   file = fopen(INPUT_FILENAME,"rt");
   while (fgets(STR_NEXT_LINE, 1000, file) != NULL)
   {
         n = 0;
         SET_NEW_LINE_NUMBER();
       //  DRAW_LINE_NUMBER();
         TRIM_STRING(); // Eliminate all the starting white spaces //
         PARSE_STRING();
         if (STACK_INDEX > 1)
         {
            printf ("Line number %d: ",Line_Number);
            DRAW_LINE_NUMBER();
            PRINT_STACK_CONTENTS(); 
         }
         EVALUATE_STACK(); // Checks if the line is gramatically correct or not. posts "ACCEPT" if it is, and then if it is grammatically incorrect, posts "REJECT"
         if (STACK_INDEX > 0)
         {
                         DRAW_NEW_LINE();
                         DRAW_NEW_LINE();
         }
        // DRAW_NEW_LINE();
         CLEAR_STACK();
       //  DRAW_NEW_LINE();
   }
    
   #ifdef TAKE_INPUT_FROM_TEXT_FILE
   fclose(file);
   fclose(pFile);
   #endif
   if (BRACKET_INDEX > 0)
   {
      printf ("FATAL ERROR! Unterminated group in the code detected. Missing '}'\n");
      HAS_ERROR = true;
      fprintf (pErrorFile,"\n");
      fprintf (pErrorFile,"FATAL ERROR!\n");
      fprintf (pErrorFile,"Message: Reached end of file while parsing. Missing '}'\n");
      fprintf (pErrorFile,"\n");
      num_errors++;
   }
   
   if (HAS_ERROR)
      printf ("ERROR DETECTED IN PARSING!\n");
   else
      printf ("Success!\n");
  // getch();
   fclose(pErrorFile);
       if (!HAS_ERROR)
       {
                      func_index =0;
                      system("interpreter.exe TRUE");
                      FILE *file = fopen("Code.art","rt");
                      FILE *outfile = fopen("Syntax.art","wt");
                      String temporary_string;
                      while (fgets(STR_NEXT_LINE, 1000, file) != NULL)
                      {
                            strcpy(temporary_string,STR_NEXT_LINE);
                            if (temporary_string[strlen(temporary_string)-1] == '\n')
                               temporary_string[strlen(temporary_string)-1] = '\0';
                            PARSE_STRING();
                            fprintf(outfile,temporary_string);
                            fprintf(outfile,"\n");
                            for (int i = 0;i < STACK_INDEX;i++)
                                fprintf(outfile,"%s ",Stack[i]);
                            fprintf(outfile,"\n");
                            CLEAR_STACK();
                      }
                      fclose(file);
                      fclose(outfile);
              //        printf ("Okay!\n");
              //        getch();
               //       main_interpreter();
       }
       else
       {
                      system("cls");
                      FILE *error = fopen(ERROR_LOG,"rt");
                      String temporary_string;
                      int cc = 0;
                      
                      while (fgets(STR_NEXT_LINE, 1000, file) != NULL)
                      {
                            strcpy(temporary_string,STR_NEXT_LINE);
                            if (temporary_string[strlen(temporary_string)-1] == '\n')
                               temporary_string[strlen(temporary_string)-1] = '\0';
                            if (cc % 2 == 0 || cc % 3 == 0)
                               printf ("%s\n",temporary_string);
                            cc++;
                      }
                      fclose(error);
                      printf ("\n");
                      printf ("Errors detected: %d\n",num_errors);
                      printf ("\n");
                      system("interpreter.exe FALSE");
       }
   /* Semantic analyzer */
   if (!HAS_ERROR)
   {
				  int statement_index = 0;
				  int function_evaluating_index = 0;
				  int bFunction = 0;
				  int function_type;
				  int num_braces = 0;
				  int numParameters = 0;
				  int index;
				  CLEAR_FUNCTION_LIST();
				  String first_token;
				  String function_name;
				  String cand;
		          FILE *file = fopen("Syntax.art","rt");
                  String temporary_string,temporary_string2;
                  while (fgets(STR_NEXT_LINE, 1000, file) != NULL)
                  {
                        fgets(temporary_string2, 1000, file);
                            if (temporary_string2[strlen(temporary_string2)-1] == '\n')
							   temporary_string2[strlen(temporary_string2)-1] = '\0';
				  		strcpy(temporary_string,STR_NEXT_LINE);
						if (temporary_string[strlen(temporary_string)-1] == '\n')
							temporary_string[strlen(temporary_string)-1] = '\0';
						if (strlen(temporary_string) == 0)
						   continue;
			             strcpy(STR_NEXT_LINE,temporary_string);
			             PARSE_STRING();
						if (bFunction == 0)
						{
							GET_FIRST_TOKEN(temporary_string,first_token);
							if (strcmp("function",first_token) == 0 || strcmp("main",first_token) == 0 || strcmp("main()",first_token) == 0 || strcmp("main ()",first_token) == 0)
							{
								if (strcmp("function",first_token) == 0)
									Dissect_Function_Name(temporary_string,function_name);
								else
									strcpy(function_name,"main");
								numParameters = countParameters(temporary_string);
								function_evaluating_index = ADD_FUNCTION(function_name);
								if (strcmp(function_name,"main") != 0)
								   function_type = getFunctionType (temporary_string);
                                if (strcmp(function_name,"main") != 0)
								   FUNCTIONS[function_evaluating_index].return_type = function_type;
								FUNCTIONS[function_evaluating_index].num_parameters = numParameters;
								if (numParameters > 0)
								   populate_parameters(function_evaluating_index,temporary_string);
								bFunction = 1;
							//	printf ("Function: %s returns %d\n",FUNCTIONS[function_evaluating_index].name,FUNCTIONS[function_evaluating_index].return_type);
							//	getch();
							}
						}
						else
						{
							index = FUNCTIONS[function_evaluating_index].num_statements;
							strcpy(FUNCTIONS[function_evaluating_index].statements[index],temporary_string);
							index++;
							FUNCTIONS[function_evaluating_index].num_statements++;
							strcpy(FUNCTIONS[function_evaluating_index].statements[index],temporary_string2);
							index++;
							GET_FIRST_TOKEN(temporary_string2,cand);
							if (strcmp(cand,"FUNCTION_CALL") == 0)
							{
                                   if (!Evaluate_Call(temporary_string))
                                   {
                                         printf ("%s - Parameters not sufficient or excess to call the function\n",temporary_string);
                                         HAS_ERROR = true;
                                   }
                            }
                            else if (strcmp(cand,"return_optr") == 0 && FUNCTIONS[function_evaluating_index].return_type == -1)
                            {
                                  printf ("%s - Returning not permitted in void functions\n",temporary_string);
                                  HAS_ERROR = true;
                            }
                            else if (strcmp(cand,"assign_optr") == 0)
                            {
                                 if (!Evaluate_Assign(temporary_string))
                                 {
                                       printf ("%s - Address to function assigned with invalid address conversion type\n",temporary_string);
                                       HAS_ERROR = true;
                                 }
                            }
							FUNCTIONS[function_evaluating_index].num_statements++;
							if (strcmp(temporary_string,"{") == 0)
								num_braces++;
							else if (strcmp(temporary_string,"}") == 0)
								num_braces--;
							if (num_braces == 0)
								bFunction = 0;
						}
						//if (HAS_ERROR)
					//	printf ("errored\n");
				//		getch();
                  }
                  fclose(file);
                  if (!HAS_ERROR)
                  {
                      printf ("\n");
                      printf ("Semantically analyzed successfully\n");
                      system("cls");
                        if (argc > 2)
                           IMPROVED_WATCH_AND_TRACE = true;
                        else if (argc > 1)
                           WATCH_AND_TRACE = true;
                      main_interpreter();
                  }
                  else
                  {
                      printf ("\n");
                      printf ("ERROR IN SEMANTICALLY ANALYZING\n");
                  }
   }
   #ifndef TESTING_FUNCTIONS
   if (!HAS_ERROR)
   {
       int aa;
       String funcName;
       do
       {
              system ("cls");
              printf ("Enter name of the function: ");
              gets(funcName);
              aa = FUNCTION_AVAILABLE(funcName);
              if (aa == -1)
                 printf ("Function non-existant\n");
              else
              {
                  printf ("Following statements will be executed: \n");
                  for (int i = 2;i < FUNCTIONS[aa].num_statements-2;i+=2)
                  {
                      printf ("%s\n",FUNCTIONS[aa].statements[i]);
                  }
                  printf ("\n");
              }
       } while (strcmp(funcName,"END") != 0);
   }
   #endif
   getch();
   
   
   /* We should make the program wait for a user to press a key if the output is written
   to standard output, that way the user should be able to get a glimpse of the output
   from the standard console before the program exits */
   #ifndef WRITE_OUTPUT_TO_TEXT_FILE
   getch();
   #endif
   /* End the main loop: Terminate the scanner */
}
