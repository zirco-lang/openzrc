#include <parser.h>
#include <util.h>

#include <stdio.h>
#include <string.h>


token * initial_token;
int total_tokens = 0;

void print_indent(FILE * out, int num) {
    for (int i = 0; i < num*2; i++) {
        fprintf(out, " ");
    }
}
void print_expr(FILE * out, parse_expr_t t, int in);

void print_type(FILE * out, parse_type_t t, int in) {
    print_indent(out, in);
    fprintf(out, "type[\n");
    print_indent(out, in+1);
    fprintf(out, "num_ptr = {%d}\n", t.num_ptr);
    print_indent(out, in+1);
    fprintf(out, "name = {\n");
    print_indent(out, in+2);
    print_token(out, *t.name);
    print_indent(out, in+1);
    fprintf(out, "}\n");
    print_indent(out, in);
    fprintf(out, "]");
    
}

void print_define(FILE * out, parse_define_t t, int in) {
    print_indent(out, in);
    fprintf(out, "define[\n");
    print_indent(out, in+1);
    fprintf(out, "name={");
    print_token(out, *t.name);
    fprintf(out, "}\n");
    if (t.typ) {
        
        print_indent(out, in+1);
        fprintf(out, "type={");
        print_type(out, *t.typ, in+2);
        fprintf(out, "\n");
        print_indent(out, in+1);
        fprintf(out, "}\n");
    }
    print_indent(out, in);
    fprintf(out, "]");
}

void print_arg(FILE * out, parse_arg_t t, int in) {
    print_indent(out, in);
    fprintf(out, "arg[\n");
    print_indent(out,in+1);
    fprintf(out, "type={%d}\n", t.typ);
    
    if (t.typ == 1) {
        print_indent(out,in+1);
        fprintf(out, "define={\n");
        print_define(out, t.def, in+2);
        print_indent(out,in+1);
        fprintf(out, "}\n");
        
    }
    print_indent(out,in);
    fprintf(out,"]");
}

void print_fn_call(FILE* out, parse_fn_call_t t, int in) {
    print_indent(out, in);
    fprintf(out, "fncall[\n");
    print_indent(out, in+1);
    fprintf(out, "name={");
    print_token(out, (token)*t.name);
    fprintf(out, "}\n");
    print_indent(out, in+1);
    fprintf(out, "args={\n");
    for (int i = 0; i < t.num_args; i++) {
        print_expr(out, t.args[i], in+2);
        fprintf(out, "\n");
    }
    print_indent(out, in+1);
    fprintf(out, "}\n");
    print_indent(out, in);
    fprintf(out, "];");
}


void print_scope(FILE * out, parse_scope_t t, int in) {
    print_indent(out, in);
    fprintf(out, "scope[\n");
    print_indent(out, in+1);
    fprintf(out, "data={\n");
    for (int i = 0; i < t.num_expr; i++) {
        print_expr(out, t.expr[i], in+2);
        fprintf(out, "\n");
    }
    print_indent(out, in+1);
    fprintf(out, "}\n");
    print_indent(out, in);
    fprintf(out, "];\n");
}

void print_expr(FILE * out, parse_expr_t t, int in) {
    print_indent(out, in);
    fprintf(out, "expr[\n");
    print_indent(out, in+1);
    fprintf(out, "type={%s}\n", parse_expr_names[t.type]);
    print_indent(out, in+1);
    fprintf(out, "data={\n");
    
    switch (t.type) {
        case ET_LITERAL: {
            print_indent(out, in+2);
            print_token(out, *((token*)t.data));
            break;
        }
        case ET_FN_CALL: {
            print_fn_call(out, *(parse_fn_call_t*)t.data, in+2);
            break;
        }
        case ET_RETURN: {
            print_expr(out, *((parse_expr_t*)t.data), in+2);
            break;
        }
        case ET_SCOPE: {
            print_scope(out, *((parse_scope_t*)t.data), in+2);
            break;
        }
        default:
            print_indent(out, in+2);
            fprintf(out, "unknown");
            break;
    }
    fprintf(out, "\n");
    print_indent(out, in+1);
    fprintf(out, "}\n");
    print_indent(out, in);
    fprintf(out, "];");
}

void print_fn(FILE* out, parse_fn_t t, int in) {
    print_indent(out, in);
    fprintf(out, "fn_def[\n");
    print_indent(out, in+1);
    fprintf(out, "name={\n");
    print_indent(out, in+2);
    print_token(out, *t.name);
    fprintf(out, "\n");
    print_indent(out, in+1);
    fprintf(out, "args: {\n");
    for (int i = 0; i < t.num_args; i++) {
        print_arg(out, t.args[i], in+2);
        fprintf(out, "\n");
    }
    print_indent(out, in+1);
    fprintf(out, "}\n");
    print_indent(out, in+1);
    fprintf(out, "ret ={\n");
    print_type(out, t.ret, in+2);
    print_indent(out, in+1);
    fprintf(out, "}\n");
    print_indent(out, in+1);
    fprintf(out, "body: {\n");
    print_expr(out, t.body, in+2);
    print_indent(out, in+1);
    fprintf(out, "}\n");
    print_indent(out, in);
    fprintf(out, "]");
    
}

void print_tree(FILE * out, parse_root_t t) {
    fprintf(out, "{\n");
    for (int i = 0; i < t.num_items; i++) {
        switch (t.item[i].typ) {
            case RT_FN: {
                print_fn(out, *((parse_fn_t*)t.item[i].data), 1);
                fprintf(out, "\n");
                break;
            }
        }
    }
    fprintf(out, "}\n");
}



int parse_expr(token * tokens, parse_expr_t * out);

int parse_fn_call(token * tokens, parse_fn_call_t * out) {
    int current = 0;
    int arg = 0;
    if (tokens[0].type == LT_LABEL && tokens[1].type == LT_OPEN_PAREN) {
        current = 2;
        while (tokens[current].type != LT_CLOSE_PAREN) {
            current += parse_expr(tokens+current, out->args+arg);
            if (tokens[current].type == LT_COMMA) {
                current++;
            }
            arg++;
        }
        current++;
        out->name = tokens;
        out->num_args = arg;
    } else {
        printf("Error: Not a function call!: ");
        print_token(stdout, tokens[0]);
        printf(", ");
        print_token(stdout, tokens[0]);
        printf("\n");
        quit(1);
    }
    return current;
}

int parse_scope(token * tokens, parse_scope_t * out) {
    int ret = 0;
    if (tokens[0].type == LT_OPEN_CURLY) {
        ret++;
        int num_expr = 0;
        while (tokens[ret].type != LT_CLOSE_CURLY && tokens+ret < initial_token + total_tokens) {
            ret += parse_expr(tokens+ret, out->expr+num_expr);
            num_expr++;
        }
        ret++;
        out->num_expr = num_expr;
    }
    return ret;
}

int parse_type(token * tokens, parse_type_t * out) {
    int ret = 0;
    int num_ptr = 0;
    while (tokens[ret].type == LT_ASTERISK) {
        num_ptr++;
        ret++;
    }
    if (tokens[ret].type == LT_LABEL) {
        out->num_ptr = num_ptr;
        out->name = tokens+ret;
        ret++;
    } else {
        printf("Expected type name, got: ");
        print_token(stdout, tokens[ret]);
        quit(1);
    }
    return ret;
}

int parse_define(token * tokens, parse_define_t * out) {
    int ret = 0;
    if (tokens[ret].type == LT_LABEL) {
        out->name = tokens+ret;
        ret++;
        if (tokens[ret].type == LT_COLON) {
            ret++;
            parse_type_t * type = add_alloc(sizeof(parse_type_t)); 
            ret += parse_type(tokens+ret, type);
            out->typ = type;
        } else {
            out->typ = 0;
        }
    } else {
        printf("Error: A define should start with a label!\n");
        print_token(stdout, tokens[0]);
        quit(1);
    }
    return ret;
}

int parse_expr(token * tokens, parse_expr_t * out) {
    int i = 0;
    if (tokens[i].type == LT_LABEL) {
        if (strcmp((char*)tokens[i].data, "return") == 0) {
            out->type = ET_RETURN;
            parse_expr_t * ret_expr = add_alloc(sizeof(parse_expr_t));
            i += parse_expr(tokens+1, ret_expr);
            out->data = (void*) ret_expr;
        } else if (strcmp((char*)tokens[i].data, "let") == 0) {
            out->type = ET_DEFINE;
            parse_define_t * define = add_alloc(sizeof(parse_define_t));
            i += parse_define(tokens+1, define);
            out->data = (void*) define;
        } else if (tokens[i+1].type == LT_OPEN_PAREN) {
            parse_fn_call_t * fn_call = add_alloc(sizeof(parse_fn_call_t));
            i += parse_fn_call(tokens, fn_call);
            out->type = ET_FN_CALL;
            out->data = (void*) fn_call;
        }
    }
    else if (tokens[i].type == LT_OPEN_CURLY) {
        out->type = ET_SCOPE;
        parse_scope_t * ret_scope = add_alloc(sizeof(parse_scope_t));
        i += parse_scope(tokens, ret_scope);
        out->data = (void*) ret_scope;
    }
    else if (tokens[i].type == LT_STRING_LITERAL || tokens[i].type == LT_INT_LITERAL) {
        out->type = ET_LITERAL;
        out->data = (void*) tokens;
        i++;
    }
    
    if (tokens[i].type == LT_SEMICOLON) {
        i++;
    } 
    return i;
}

int parse_arg(token * tokens, parse_arg_t * out) {
    int ret = 0;
    int check = tokens[0].type == LT_DOT;
    for (int i = 1; i < 3; i++) {
        check = check && tokens[i].type == LT_DOT;
    }
    if (check) {
        ret += 3;
        out->typ = 0;
    } else {
        out->typ = 1;
        ret += parse_define(tokens, &out->def);
    }
    return ret;
}

int parse_fn(token * tokens, parse_fn_t * out) {
    int ret = -1;
    if (tokens[0].type == LT_LABEL && strcmp((char*)tokens[0].data, "fn") == 0) {
        ret = 1;
        if (tokens[ret].type == LT_LABEL && tokens[ret+1].type == LT_OPEN_PAREN) {
            out->name = tokens+ret;
            ret += 2;
            int arg_num = 0;
            while (tokens[ret].type != LT_CLOSE_PAREN) {
                print_token(stdout, tokens[ret]);
                ret += parse_arg(tokens+ret,out->args+arg_num);
                arg_num++;
                if (tokens[ret].type == LT_COMMA) {
                    ret++;
                }
            }
            out->num_args = arg_num;
            ret++;
            if (tokens[ret].type == LT_HYP && tokens[ret+1].type == LT_GREATER) {
                ret += 2;
                ret += parse_type(tokens+ret, &out->ret);
                ret += parse_expr(tokens+ret, &out->body);
            } else {
                printf("No return type for function. Got: ");
                print_token(stdout, tokens[ret]);
                quit(1);
            }
        }
    } else {
        printf("Not a function definition! Got: ");
        print_token(stdout, tokens[0]);
        printf("\n");
        quit(1);
    }
    return ret;
}

int parse_root(token * tokens, parse_root_t * out) {
    int ret = 0;
    int i = 0;
    while (tokens+ret < initial_token + total_tokens) {
        if (tokens[ret].type == LT_LABEL && strcmp((char*)tokens[ret].data, "fn") == 0) {

            out->item[i].typ = RT_FN;
            parse_fn_t * fn = add_alloc(sizeof(parse_fn_t));
            ret += parse_fn(tokens+ret, fn);
            out->item[i].data = (void*) fn;
            i++;
        }
    }
    out->num_items = i;
    return ret;
}

parse_root_t * parse(token * tokens, int num_tokens) {
    initial_token = tokens;
    total_tokens = num_tokens;
    parse_root_t * out = (parse_root_t*)add_alloc(sizeof(parse_root_t));
    parse_root(tokens, out);
    return out;
}