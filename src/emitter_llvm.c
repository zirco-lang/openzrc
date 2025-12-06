#include <emitter_llvm.h>
#include <lexer.h>
#include <util.h>
#include <string.h>
#include <stdlib.h>

void gen_header(char * file_name, FILE * out) {
    int ret;
    for (ret = (strlen(file_name) - 1); ret >= 0; ret--) {
        if (file_name[ret] == '/') {
            ret++;
            break;
        }
    }
    char * sub = alloc_substring(strlen(file_name)-ret, file_name, ret);
    fprintf(out, "; ModuleID = '%s'\n", sub);
    fprintf(out, "source_filename = \"%s\"\n", sub);
}

token * root_literals;
int root_num_literals;

void preprocess_strings(FILE * out) {
    int count = 0;
    for (int i = 0; i < root_num_literals; i++) {
        if (root_literals[i].type == LT_STRING_LITERAL) {
            root_literals[i].data_len = count;

            char * original = (char*)root_literals[i].data;
            char tmp[256];
            int len = 0;
            int j;
            int k = 0;
            for (j = 1; j < strlen(original)-1; j++) {
                if (original[j] == '\\' && original[j+1] == 'n') {
                    tmp[k] = '\\';
                    tmp[k+1] = '0';
                    tmp[k+2] = 'A';
                    k += 3;
                    j++;
                    len++;
                } else {
                    tmp[k] = original[j];
                    k++;
                    len++;
                }
            }
            tmp[k] = '\\';
            tmp[k+1] = '0';
            tmp[k+2] = '0';
            tmp[k+3] = 0;
            len++;
            fprintf(out, "@str.%d = private unnamed_addr constant [%d x i8] c\"%s\", align 1\n", count, len, tmp);
            count++;
        }
    }
    
}

int llvm_size(char * typ) {
    if (strcmp(typ, "ptr") == 0) return 8;
    else return atoi(typ+1)/8;

}

char * fn_name[256];
char * fn_ret[256];
char * fn_args[256];
int num_fns = 0;
int current_function;
int get_fn(char * name) {
    for (int i = 0; i < num_fns; i++) {
        if (strcmp(fn_name[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

int calls = 0;


void process_expr(parse_expr_t expr, FILE * out) {
    switch (expr.type) {
        case ET_SCOPE: {
            parse_scope_t * scope = (parse_scope_t*) expr.data;
            for (int i = 0; i < scope->num_expr; i++) {
                process_expr(scope->expr[i], out);
            }
            break;
        }
        case ET_RETURN: {
            parse_expr_t * data = (parse_expr_t*) expr.data;
            if (data->type == ET_LITERAL) {
                token * lit = (token *) data->data;
                if (root_literals[lit->data_len].type == LT_INT_LITERAL) {
                    fprintf(out, "  ret %s %s\n", fn_ret[current_function], (char *)root_literals[lit->data_len].data);
                } else {
                    printf("we don't know how to handle this yet!\n");
                    fclose(out);
                    quit(1);
                }
            } else {
                printf(" we don't know how to handle this yet!");
                fclose(out);
                quit(1);
            }
            break;
        }
        case ET_FN_CALL: {
            char * args[32];
            parse_fn_call_t * fn = (parse_fn_call_t *) expr.data;
            for (int i = 0; i < fn->num_args; i++) {
                parse_expr_t * arg = fn->args+i;
                switch (arg->type) {
                    case ET_LITERAL: {
                        token * lit = (token * )arg->data;
                        token root = root_literals[lit->data_len];
                        char tmp[256] = "";
                        if (root.type == LT_INT_LITERAL) {
                            printf("i don't know what to do here yet. Parsing int literal in function call.\n");
                            fclose(out);
                            quit(1);
                        } else if (root.type == LT_STRING_LITERAL) {
                            sprintf(tmp, "ptr @str.%d", root.data_len);
                        }
                        args[i] = alloc_substring(strlen(tmp), tmp, 0);
                        break;
                    }
                    default: {
                        printf("we don't know how to handle anything other than literals in llvm generation.\n");
                        fclose(out);
                        quit(1);
                        break;
                    }
                }
            }

            int fn_i = get_fn((char*)fn->name->data);
            if (fn_i >= 0) {

            fprintf(out, "  %%call.%d = call %s %s @%s(", calls, fn_ret[fn_i], fn_args[fn_i], fn_name[fn_i]);
            for (int i = 0; i < fn->num_args; i++) {
                fprintf(out, "%s", args[i]);
                if (i < fn->num_args - 1) {
                    fprintf(out, ", ");
                }
            }
            fprintf(out, ")\n");
            calls++;
            }            
            break;
        }
        default: {
            printf("We don't know how to handle this expression yet!\n");
            fclose(out);
            quit(1);
            break;
        }
    }
}


void arg_typ(parse_arg_t t, char ** typ) {
    *typ = "ptr";
    switch (t.typ) {
                case 0: {
                    *typ = "...";
                    break;
                }
                case 1: {
                    if (t.def.typ->num_ptr == 0) {
                        *typ = ((char*)t.def.typ->name->data);
                        if (strcmp(*typ, "usize") == 0) {
                            *typ = "i64";
                        }
                    }
                    break;
                }
            }
}

void process_fn(parse_fn_t t, FILE * out) {
    fprintf(out, "\n");
    char * ret = t.ret.num_ptr > 0 ? "ptr" : (char*)t.ret.name->data;
    int add_d = 0;
    if (get_fn((char*)t.name->data) == -1) { 
        fn_name[num_fns] = (char*)t.name->data;
        fn_ret[num_fns] = (char*) alloc_substring(strlen(ret), ret, 0);
        current_function = num_fns;
        num_fns++;
        add_d = 1;
    }
    if (t.body.type == ET_UNKNOWN) {
        fprintf(out, "declare");
    } else {
        fprintf(out, "define");
    }
        
        fprintf(out, " %s @%s", ret, (char*)t.name->data);
        char args[1024] = "(";
        for (int i = 0; i < t.num_args; i++) {
            char * typ = "";
            arg_typ(t.args[i], &typ);
            strcat(args, typ);

            if (t.body.type != ET_UNKNOWN) {
                char t[16];
                sprintf(t, " %%%d", i);
                strcat(args, t);
            }
            if (i < t.num_args-1) {
                strcat(args, ", ");
            }
        }
        strcat(args, ")");
        if (add_d) {
            fn_args[num_fns-1] = alloc_substring(strlen(args), args, 0);
        }
        fprintf(out, "%s", args);
    if (t.body.type != ET_UNKNOWN) {
        fprintf(out, " {\nentry:\n");
        for (int i = 0; i < t.num_args; i++) {
            char * typ = "";
            arg_typ(t.args[i], &typ);
            
            char * name = (char*)t.args[i].def.name->data;
            fprintf(out, "  %%arg_%s = alloca %s, align %d\n", name, typ, llvm_size(typ));
            int size = llvm_size(typ);
            if (strcmp(fn_name[current_function], "main") == 0 && i == 0) {
                size = 4;
            }
            fprintf(out, "  store %s %%%d, ptr %%arg_%s, align %d\n", typ, i, name, size);
        }
        process_expr(t.body, out);
        fprintf(out, "}");
    }
    fprintf(out, "\n");
}

void process_ast(parse_root_t ast, FILE * out) {
    for (int i = 0; i < ast.num_items; i++) {
        switch (ast.item[i].typ) {
            case RT_FN: {
                process_fn(*((parse_fn_t*)ast.item[i].data), out);
                break;
            }
        }
    }
}

int gen_llvm(parse_root_t ast, char * original_file, FILE * out, token * literals, int num_literals) {
    gen_header(original_file, out);
    fprintf(out, "\n");
    root_literals = literals;
    root_num_literals = num_literals;
    preprocess_strings(out);
    process_ast(ast, out);
    printf("not fully implemented!\n");
    return 0;
}