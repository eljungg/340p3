#include "compiler.h"
#include "project3.h"

#include <iostream>

using namespace std;

LexicalAnalyzer lexer;

void Parser::syntax_error()
{
    cout << "SYNTAX ERROR !!!\n" << endl;
    exit(1);
}

Token Parser::peek()
{
    Token t = lexer.GetToken();
    lexer.UngetToken(t);
    return t;
}


vector<string> variables;

bool exists(vector<string> list, string check)
{
    bool present = false;
    for (unsigned i = 0; i < list.size(); i++)
    {
        if (list[i].compare(check) == 0)
        {
            present = true;
            break;
        }
    }
    return present;
}

int location(string find)
{
    for (unsigned i = 0; i < variables.size(); i++)
    {
        if (variables[i].compare(find) == 0)
        {
            return i;
        }
    }
}

struct InstructionNode* parse_generate_intermediate_representation()
{
    
    Parser parser;
    struct InstructionNode* program = new InstructionNode();
    program = parser.parse_program();
    cout << "damn";
    return program;
}

struct InstructionNode* Parser::parse_program()
{
    // var_section body inputs
    struct InstructionNode* body = new InstructionNode;
    parse_var_section();
    body = parse_body();
    parse_inputs();
    return body;
}

void Parser::parse_var_section()
{
    // id_list SEMICOLON
    variables = parse_id_list();

    for (unsigned i = 0; i < variables.size(); i++)
    {
        mem[next_available] = 0;
        next_available++;
    }

    Token t = lexer.GetToken();

    if (t.token_type != SEMICOLON)
    {
        syntax_error();
    }

}

vector<string> Parser::parse_id_list()
{
    // ID COMMA id_list | ID
    Token t = lexer.GetToken();
    vector<string> list;
    vector<string> temp;

    if (t.token_type != ID)
    {
        syntax_error();
    }
    if (!exists(list, t.lexeme))
    {
        list.push_back(t.lexeme);
    }
    t = peek();
    if (t.token_type == COMMA)
    {
        t = lexer.GetToken();
        temp = parse_id_list();
        for (unsigned i = 0; i < list.size(); i++)
        {
            for (unsigned j = 0; j < temp.size(); j++)
            {
                if (!exists(list, temp[j]))
                {
                    list.push_back(temp[j]);
                }
            }
        }
        return list;
    }
    else if (t.token_type == SEMICOLON)
    {
        return list;
    }
    else
    {
        syntax_error();
    }
}

struct InstructionNode* Parser::parse_body()
{
    // LBRACE stmt_list RBRACE
    struct InstructionNode* instl;
    Token t = lexer.GetToken();
    if (t.token_type != LBRACE)
    {
        syntax_error();
    }
    instl = parse_stmt_list();
    t = lexer.GetToken();
    if (t.token_type != RBRACE)
    {
        syntax_error();
    }
    return instl;
}

struct InstructionNode* Parser::parse_stmt_list()
{
    // stmt stmt_list | stmt
    struct InstructionNode* instl1;
    struct InstructionNode* instl2;

    instl1 = parse_stmt();
    Token t = peek();
    if (t.token_type == ID || t.token_type == WHILE
        || t.token_type == IF || t.token_type == SWITCH
        || t.token_type == FOR || t.token_type == INPUT
        || t.token_type == OUTPUT)
    {
        instl2 = parse_stmt_list();
        struct InstructionNode* temp;
        // append instl2 to the end of instl1
        temp = instl1;
        while (temp->next)
        {
            temp = temp->next;
        }
        temp->next = instl2;
        return instl1;
    }
    else if (t.token_type == RBRACE)
    {
        return instl1;
    }
    else
    {
        syntax_error();
    }
}

struct InstructionNode* Parser::parse_stmt()
{
    // assign_stmt | while_stmt | if_stmt | switch_stmt | for_stmt
    // output_stmt | input_stmt
    struct InstructionNode* inst = new InstructionNode;
    Token t = peek();
    if (t.token_type == ID)
    {
        inst = parse_assign_stmt();
    }
    else if (t.token_type == WHILE)
    {
        inst = parse_while_stmt();
    }
    else if (t.token_type == IF)
    {
        inst = parse_if_stmt();
    }
    else if (t.token_type == SWITCH)
    {
        inst = parse_switch_stmt();
    }
    else if (t.token_type == FOR)
    {
        inst = parse_for_stmt();
    }
    else
    {
        syntax_error();
    }
    return inst;
}

struct InstructionNode* Parser::parse_assign_stmt()
{
    // ID EQUAL primary SEMICOLON
    // ID EQUAL expr SEMICOLON

    struct InstructionNode* inst = new InstructionNode;
    struct InstructionNode* temp = new InstructionNode;

    inst->type = ASSIGN;
    Token t = lexer.GetToken();
    if (t.token_type != ID)
    {
        syntax_error();
    }
    string to_store = t.lexeme;
    inst->assign_inst.left_hand_side_index = location(to_store);
    
    t = lexer.GetToken();
    if (t.token_type != EQUAL)
    {
        syntax_error();
    }
    Token t1 = lexer.GetToken();
    Token t2 = lexer.GetToken();
    if (t1.token_type == ID || t1.token_type == NUM)
    {
        if (t2.token_type == PLUS || t2.token_type == MINUS
            || t2.token_type == MULT || t2.token_type == DIV)
        {
            lexer.UngetToken(t2);
            lexer.UngetToken(t1);
            temp = parse_expr();
            inst->assign_inst.op = temp->assign_inst.op;
            inst->assign_inst.operand1_index = temp->assign_inst.operand1_index;
            inst->assign_inst.operand2_index = temp->assign_inst.operand2_index;

            // evaluate the right hand side and set the value of the lhs to the result
            int result;
            int value1 = mem[inst->assign_inst.operand1_index];
            int value2 = mem[inst->assign_inst.operand2_index];
            ArithmeticOperatorType a_op = inst->assign_inst.op;
            
            if (a_op == OPERATOR_PLUS)
            {
                result = value1 + value2;
            }
            else if (a_op == OPERATOR_MINUS)
            {
                result = value1 - value2;
            }
            else if (a_op == OPERATOR_MULT)
            {
                result = value1 * value2;
            }
            else if (a_op == OPERATOR_DIV)
            {
                result = value1 / value2;
            }
            mem[inst->assign_inst.left_hand_side_index] = result;
            
            
        }
        else if (t2.token_type == SEMICOLON)
        {
            // This is an assign statement in the form a = 0; / a = b;
            lexer.UngetToken(t2);
            lexer.UngetToken(t1);
            temp = parse_primary();
            // set the insts' operand1 index to temp's operand1 index 
            inst->assign_inst.operand1_index = temp->assign_inst.operand1_index;
            // assign the value located in operand1's index to th value associated with the lhs index
            mem[inst->assign_inst.left_hand_side_index] = mem[inst->assign_inst.operand1_index]; 

            inst->assign_inst.op = OPERATOR_NONE;
        
        }
        else
        {
            syntax_error();
        }
        
    }
    t = lexer.GetToken();
    if (t.token_type != SEMICOLON)
    {
        syntax_error();
    }
    return inst;
}

struct InstructionNode* Parser::parse_expr()
{
    // primary op primary
    struct InstructionNode* inst = new InstructionNode;
    struct InstructionNode* temp = new InstructionNode;
    temp = parse_primary();
    inst->assign_inst.operand1_index = temp->assign_inst.operand1_index;

    int op = parse_op();
    if (op == OPERATOR_PLUS)
    {
        inst->assign_inst.op = OPERATOR_PLUS;
    }
    else if (op == OPERATOR_MINUS)
    {
        inst->assign_inst.op = OPERATOR_MINUS;
    }
    else if (op == OPERATOR_MULT)
    {
        inst->assign_inst.op = OPERATOR_MULT;
    }
    else if (op == OPERATOR_DIV)
    {
        inst->assign_inst.op = OPERATOR_DIV; 
    }
    else
    {
        inst->assign_inst.op = OPERATOR_NONE;
    }
    

    temp = parse_primary();
    inst->assign_inst.operand2_index = temp->assign_inst.operand2_index;

    return inst;
}

struct InstructionNode* Parser::parse_primary()
{
    // ID | NUM
    struct InstructionNode* temp = new InstructionNode;
    Token t = lexer.GetToken();
    if (t.token_type == ID)
    {
        temp->assign_inst.operand1_index = location(t.lexeme);
    }
    else if (t.token_type == NUM)
    {
        if (!exists(variables, t.lexeme))
        {
            variables.push_back(t.lexeme); 
            mem[next_available] = stoi(t.lexeme); 
            next_available++;
        }
        
        temp->assign_inst.operand1_index = location(t.lexeme);
    }
    else
    {
        syntax_error();
    }
    return temp;
}

int Parser::parse_op()
{
    // PLUS | MINUS | MULT | DIV
    Token t = lexer.GetToken();
    if (t.token_type == PLUS)
    {
        return OPERATOR_PLUS;
    }
    else if (t.token_type == MINUS)
    {
        return OPERATOR_MINUS;
    }
    else if (t.token_type == DIV)
    {
        return OPERATOR_DIV;
    }
    else if (t.token_type == MULT)
    {
        return OPERATOR_MULT;
    }    
    else
    {
        syntax_error();
    }
    

}

void Parser::parse_output_stmt()
{
    // output ID SEMICOLON
    Token t = lexer.GetToken();
    if (t.token_type != OUTPUT)
    {
        syntax_error();
    }
    t = lexer.GetToken();
    if (t.token_type != ID)
    {
        syntax_error();
    }

    cout << mem[location(t.lexeme)] << endl;
    

    t = lexer.GetToken();
    if (t.token_type != SEMICOLON)
    {
        syntax_error();
    }

}

void Parser::parse_input_stmt()
{
    // input ID SEMOCOLON
    struct InstructionNode* inst = new InstructionNode;
    

    Token t = lexer.GetToken();
    if (t.token_type != INPUT)
    {
        syntax_error();
    }
    t = lexer.GetToken();
    if (t.token_type != ID)
    {
        syntax_error();
    }
    
    inst->input_inst.var_index = location(t.lexeme);

    mem[location(t.lexeme)] = inputs[next_input];
    next_input++;

    t = lexer.GetToken();
    if (t.token_type != SEMICOLON)
    {
        syntax_error();
    }

}

struct InstructionNode* Parser::parse_while_stmt()
{
    // WHILE condition body
    struct InstructionNode* inst = new InstructionNode;
    struct InstructionNode* temp = new InstructionNode;
    Token t = lexer.GetToken();
    if (t.token_type != WHILE)
    {
        syntax_error();
    }

    inst->type = CJMP;
    temp = parse_condition();
    inst->cjmp_inst.condition_op = temp->cjmp_inst.condition_op;
    inst->cjmp_inst.operand1_index = temp->cjmp_inst.operand1_index;
    inst->cjmp_inst.operand2_index = temp->cjmp_inst.operand2_index;

    inst->next = parse_body();
    struct InstructionNode* jmpNode = new InstructionNode;
    jmpNode->type = JMP;
    jmpNode->next = nullptr;
    jmpNode->jmp_inst.target = inst;

    temp = inst;
    while (temp->next)
    {
        temp = temp->next;
    }
    temp->next = jmpNode;

    struct InstructionNode* noopNode = new InstructionNode;
    noopNode->type = NOOP;
    noopNode->next = nullptr;
    jmpNode->next = noopNode;
    inst->cjmp_inst.target = noopNode;

    return inst;
}

struct InstructionNode* Parser::parse_if_stmt()
{
    // IF condition body
    struct InstructionNode* inst = new InstructionNode;
    struct InstructionNode* temp = new InstructionNode;
    Token t = lexer.GetToken();
    if (t.token_type != IF)
    {
        syntax_error();
    }

    inst->type = CJMP;
    temp = parse_condition();
    inst->cjmp_inst.condition_op = temp->cjmp_inst.condition_op;
    inst->cjmp_inst.operand1_index = temp->cjmp_inst.operand1_index;
    inst->cjmp_inst.operand2_index = temp->cjmp_inst.operand2_index;

    inst->next = parse_body(); 

    struct InstructionNode* noopNode = new InstructionNode;
    noopNode->next = nullptr;
    noopNode->type = NOOP;

    temp = inst;
    while (temp->next)
    {
        temp = temp->next;
    }
    temp->next = noopNode;
    inst->cjmp_inst.target = noopNode;

    return inst;
}

struct InstructionNode* Parser::parse_condition()
{
    // primary relop primary
    struct InstructionNode* inst = new InstructionNode;
    struct InstructionNode* temp = new InstructionNode;
    temp = parse_primary();
    inst->cjmp_inst.operand1_index = temp->cjmp_inst.operand1_index;

    temp = parse_relop();
    inst->cjmp_inst.condition_op = temp->cjmp_inst.condition_op;

    temp = parse_primary();
    inst->cjmp_inst.operand2_index = temp->cjmp_inst.operand2_index;

    return inst;

}

struct InstructionNode* Parser::parse_relop()
{
    // GREATER | LESS | NOTEQUAL
    struct InstructionNode* inst = new InstructionNode;
    Token t = lexer.GetToken();
    if (t.token_type == GREATER)
    {
        inst->cjmp_inst.condition_op = CONDITION_GREATER;
    }
    else if (t.token_type == LESS)
    {
        inst->cjmp_inst.condition_op = CONDITION_LESS;
    }
    else if (t.token_type == NOTEQUAL)
    {
        inst->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
    }
    else
    {
        syntax_error();
    }
    return inst;

}

struct InstructionNode* Parser::parse_switch_stmt()
{
    // SWITCH ID LBRACE case_list RBRACE
    // SWITCH ID LBRACE case_list default_case RBRACE
    struct InstructionNode* inst = new InstructionNode;
    Token t = lexer.GetToken();
    if (t.token_type != SWITCH)
    {
        syntax_error();
    }
    t = lexer.GetToken();
    if (t.token_type != ID)
    {
        syntax_error();
    }
    t = lexer.GetToken();
    if (t.token_type != LBRACE)
    {
        syntax_error();
    }
    parse_case_list();
    t = peek();
    if (t.token_type == RBRAC)
    {
        t = lexer.GetToken();
        return inst;
    }
    else if (t.token_type == DEFAULT)
    {
        parse_default_case();
        t = lexer.GetToken();
        if (t.token_type != RBRACE)
        {
            syntax_error();
        }
        return inst;
    }
}

struct InstructionNode* Parser::parse_case_list()
{
    // case case_list | case
    struct InstructionNode* caseNode = new InstructionNode;
    struct InstructionNode* temp = new InstructionNode;
    struct InstructionNode* jmpNode = new InstructionNode;
    caseNode = parse_case();
    temp = caseNode;
    while (temp->next)
    {
        temp = temp->next;
    }
    temp->next = jmpNode;

    Token t = peek();
    if (t.token_type == CASE)
    {
        parse_case_list();
    }
    else if (t.token_type == RBRACE)
    {
        return caseNode;
    }
    else if (t.token_type == DEFAULT)
    {
        return caseNode;
    }
}

struct InstructionNode* Parser::parse_case()
{
    // CASE NUM COLON body
    struct InstructionNode* body = new InstructionNode;

    Token t = lexer.GetToken();
    if (t.token_type != CASE)
    {
        syntax_error();
    }
    t = lexer.GetToken();
    if (t.token_type != NUM)
    {
        syntax_error();
    }
    t = lexer.GetToken();
    if (t.token_type != COLON)
    {
        syntax_error();
    }
    body = parse_body();

    return body;

}

struct InstructionNode* Parser::parse_default_case()
{
    // DEFAULT COLON body
    struct InstructionNode* body = new InstructionNode;
    Token t = lexer.GetToken();
    if (t.token_type != DEFAULT)
    {
        syntax_error();
    }
    t = lexer.GetToken();
    if (t.token_type != COLON)
    {
        syntax_error();
    }
    parse_body();
    return body;
}

struct InstructionNode* Parser::parse_for_stmt()
{
    // FOR LPAREN assign_stmt condition SEMICOLON assign_stmt RPAREN body
    struct InstructionNode* inst = new InstructionNode;
    struct InstructionNode* temp = new InstructionNode;
    struct InstructionNode* condition = new InstructionNode;
    struct InstructionNode* assignment1 = new InstructionNode;
    struct InstructionNode* assignment2 = new InstructionNode;
    struct InstructionNode* body = new InstructionNode;
    struct InstructionNode* noopNode = new InstructionNode;
    struct InstructionNode* jmpNode = new InstructionNode;

    noopNode->next = nullptr;
    noopNode->type = NOOP;

    jmpNode->type = JMP;

    Token t = lexer.GetToken();
    if (t.token_type != FOR)
    {
        syntax_error();
    }
    t = lexer.GetToken();
    if (t.token_type != LPAREN)
    {
        syntax_error();
    }

    assignment1 = parse_assign_stmt();

    condition = parse_condition();
    condition->type = CJMP;
    

    t = lexer.GetToken();
    if (t.token_type != SEMICOLON)
    {
        syntax_error();
    }


    assignment2 = parse_assign_stmt();
    assignment2->type = ASSIGN;

    t = lexer.GetToken();
    if (t.token_type != RPAREN)
    {
        syntax_error();
    }
    body = parse_body();
    temp = body;
    while (temp->next)
    {
        temp = temp->next;
    }

    temp->next = assignment2;
    condition->cjmp_inst.target = noopNode;
    condition->next = body;
    assignment2->next = jmpNode;
    jmpNode->jmp_inst.target = condition;
    jmpNode->next = noopNode;
    inst = assignment1;
    inst->next = condition;

    return inst;
}

   

void Parser::parse_inputs()
{
    // num_list
    parse_num_list();

}

void Parser::parse_num_list()
{
    // NUM 
    // NUM num_list
    Token t = lexer.GetToken();
    if (t.token_type != NUM)
    {
        syntax_error();
    }
    inputs.push_back(stoi(t.lexeme));
    t = peek();
    if (t.token_type == NUM)
    {
        parse_num_list();
    }
    else if (t.token_type == END_OF_FILE)
    {
        return;
    }

}



