#include "compiler.h"
#include "parser.h"
#include "lexer.h"

#include <iostream>

using namespace std;

LexicalAnalyzer lexer;

void Parser::syntax_error()
{
    cout << "SYNTAX ERROR !!!\n" << endl;
    exit(1);
}

struct InstructionNode * parse_generate_intermediate_representation()
{
    
}

void Parser::parse_program()
{
    // var_section body inputs
    parse_var_section();
    parse_body;
    parse_inputs;
}

void Parser::parse_var_section()
{
    // id_list SEMICOLON
    parse_id_list();
    Token t = lexer.GetToken();
    if (t.token_type != SEMICOLON)
    {
        syntax_error();
    }

}

void Parser::parse_id_list()
{
    // ID COMMA id_list | ID
    Token t = lexer.GetToken();
    if (t.token_type != ID)
    {
        syntax_error();
    }
    t = peek();
    if (t.token_type == COMMA)
    {
        t = lexer.GetToken();
        parse_id_list();
    }
    else if (t.token_type == SEMICOLON)
    {
        return;
    }
    else
    {
        syntax_error();
    }
}

void Parser::parse_body()
{
    // LBRACE stmt_list RBRACE
    Token t = lexer.GetToken();
    if (t.token_type != LBRACE)
    {
        syntax_error();
    }
    parse_stmt_list();
    t = lexer.GetToken();
    if (t.token_type != RBRACE)
    {
        syntax_error();
    }
}

void Parser::parse_stmt_list()
{
    // stmt stmt_list | stmt
    parse_stmt();
    Token t = peek();
    if (t.token_type == ID || t.token_type == WHILE
        || t.token_type == IF || t.token_type == SWITCH
        || t.token_type == FOR)
    {
        parse_stmt_list();
    }
    else if (t.token_type == RBRACE)
    {
        return;
    }
    else
    {
        syntax_error();
    }
}

void Parser::parse_stmt()
{
    // assign_stmt | while_stmt | if_stmt | switch_stmt | for_stmt
    Token t = peek();
    if (t.token_type == ID)
    {
        parse_assign_stmt();
    }
    else if (t.token_type == WHILE)
    {
        parse_while_stmt();
    }
    else if (t.token_type == IF)
    {
        parse_if_stmt();
    }
    else if (t.token_type == SWITCH)
    {
        parse_switch_stmt();
    }
    else if (t.token_type == FOR)
    {
        parse_for_stmt();
    }
    else
    {
        syntax_error();
    }
}

void Parser::parse_assign_stmt()
{
    // ID EQUAL primary SEMICOLON
    // ID EQUAL expr SEMICOLON
    Token t = lexer.GetToken();
    if (t.token_type != ID)
    {
        syntax_error();
    }
    t = lexer.GetToken();
    if (t.token_type != EQUAL)
    {
        syntax_error();
    }
    Token t1 = lexer.GetToken();
    Token t2 = lexer.getToken();
    

}

void parse_expr()
{

}

void parse_primary()
{

}

void parse_op()
{

}

void parse_output_statement()
{

}

void parse_input_statement()
{

}

void parse_while_stmt()
{

}

void parse_if_stmt()
{

}

void parse_condition()
{

}

void parse_relop()
{

}

void parse_switch_stmt()
{

}

void parse_for_stmt()
{

}

void parse_case_list()
{

}

void parse_case()
{

}

void parse_default_case()
{

}

void parse_default_case()
{

}

void parse_inputs()
{

}

void parse_num_list()
{

}


