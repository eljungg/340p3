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

bool check_for_case(string case_num)
{
    bool check = false;
    for (unsigned i = 0; i < variables.size(); i++)
    {
        if (variables[i].compare(case_num) == 0)
        {
            check = true;
            break;
        }
    }
    return check;
}

struct InstructionNode* parse_generate_intermediate_representation()
{
    
    Parser parser; 
    struct InstructionNode* program = new InstructionNode();
    program = parser.parse_program();
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
        struct InstructionNode* temp = instl1;
        // append instl2 to the end of instl1
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
    else if (t.token_type == INPUT)
    {
        inst = parse_input_stmt();
    }
    else if (t.token_type == OUTPUT)
    {
        inst = parse_output_stmt();
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

           
        }
        else if (t2.token_type == SEMICOLON)
        {
            // This is an assign statement in the form a = 0; / a = b;
            lexer.UngetToken(t2);
            lexer.UngetToken(t1);
            temp = parse_primary();
            // set the insts' operand1 index to temp's operand1 index 
            inst->assign_inst.operand1_index = temp->assign_inst.operand1_index;

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
    inst->next = nullptr;
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
    inst->assign_inst.operand2_index = temp->assign_inst.operand1_index;

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

struct InstructionNode* Parser::parse_output_stmt()
{
    // output ID SEMICOLON
    struct InstructionNode* inst = new InstructionNode;
    inst->type = OUT;
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

    inst->output_inst.var_index = location(t.lexeme);

    t = lexer.GetToken();
    if (t.token_type != SEMICOLON)
    {
        syntax_error();
    }
    inst->next = nullptr;
    return inst;

}

struct InstructionNode* Parser::parse_input_stmt()
{
    // input ID SEMOCOLON
    struct InstructionNode* inst = new InstructionNode;
    inst->type = IN;
    

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

    t = lexer.GetToken();
    if (t.token_type != SEMICOLON)
    {
        syntax_error();
    }
    inst->next = nullptr;
    return inst;
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
    while (temp->next != nullptr)
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
    inst->type = CJMP;
    temp = parse_primary();
    inst->cjmp_inst.operand1_index = temp->cjmp_inst.operand1_index;

    temp = parse_relop();
    inst->cjmp_inst.condition_op = temp->cjmp_inst.condition_op;

    temp = parse_primary();
    inst->cjmp_inst.operand2_index = temp->cjmp_inst.operand1_index;

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
    struct InstructionNode* body = new InstructionNode;
    struct InstructionNode* noopNode = new InstructionNode;
    struct InstructionNode* defaultCase = new InstructionNode;
    struct InstructionNode* temp = new InstructionNode;
    struct InstructionNode* temp1 = new InstructionNode;
    int variable;

    inst->type = CJMP;
    inst->cjmp_inst.condition_op = CONDITION_NOTEQUAL;

    noopNode->type = NOOP;
    noopNode->next = nullptr;

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

    variable = location(t.lexeme);
    inst->cjmp_inst.operand1_index = variable;

    t = lexer.GetToken();
    if (t.token_type != LBRACE)
    {
        syntax_error();
    }
    body = parse_case_list();
    temp = body;
    while (temp->next)
    {
        temp->cjmp_inst.operand1_index = variable;
        temp->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
        temp1 = temp->cjmp_inst.target;
        while (temp1->next)
        {
            temp1 = temp1->next;
        }
        temp1->jmp_inst.target = noopNode;
        temp = temp->next;
    }
    temp->cjmp_inst.operand1_index = variable;
    temp->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
    temp1 = temp->cjmp_inst.target;
    while (temp1->next)
    {
        temp1 = temp1->next;
    }
    temp1->jmp_inst.target = noopNode;
    inst = body;
    
    t = peek();
    if (t.token_type == RBRAC)
    {
        t = lexer.GetToken();
        return inst;
    }
    else if (t.token_type == DEFAULT)
    {
        defaultCase = parse_default_case();
        temp = inst;
        while (temp->next)
        {
            temp = temp->next;
        }
        
        
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
    // link all cases together, and link all cases to the noopNode
    struct InstructionNode* inst = new InstructionNode;
    struct InstructionNode* caseNode = new InstructionNode;

    inst = parse_case();
     
    Token t = peek();
    if (t.token_type == CASE)
    {
        caseNode = parse_case_list(); // temp holds the next set of case nods (cjmp, body, jmp)
        inst->next = caseNode;
        return inst;
    }
    else if (t.token_type == RBRACE)
    {
        inst->next = nullptr;
        return inst;
    }
}

struct InstructionNode* Parser::parse_case()
{
    // CASE NUM COLON body
    struct InstructionNode* body = new InstructionNode;
    struct InstructionNode* inst = new InstructionNode;
    struct InstructionNode* jmpNode = new InstructionNode;
    struct InstructionNode* temp = new InstructionNode;

    inst->type = CJMP;
    inst->next = nullptr;

    jmpNode->type = JMP;
    jmpNode->jmp_inst.target = nullptr;
    jmpNode->next = nullptr;
    

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

    if (check_for_case(t.lexeme))
    {
        inst->cjmp_inst.operand2_index = location(t.lexeme);
    }
    else
    {
        inst->cjmp_inst.operand2_index = 1000;
    }

    t = lexer.GetToken();
    if (t.token_type != COLON)
    {
        syntax_error();
    }
    body = parse_body();

    temp = body;
    while (temp->next)
    {
        temp = temp->next;
    }
    temp->next = jmpNode;
    inst->cjmp_inst.target = body;
    return inst;
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
    
    body = parse_body();

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



