#include "runtime.h"
#include <stdlib.h>

TurboObject* t_Token = NULL;
TurboObject* t_impl_Token___init__(int argc, TurboObject** args);
TurboObject* t_ASTNode = NULL;
TurboObject* t_impl_ASTNode___init__(int argc, TurboObject** args);
TurboObject* t_impl_count_leading_spaces(int argc, TurboObject** args);
TurboObject* t_count_leading_spaces = NULL;
TurboObject* t_impl_is_empty_or_comment(int argc, TurboObject** args);
TurboObject* t_is_empty_or_comment = NULL;
TurboObject* t_Lexer = NULL;
TurboObject* t_impl_Lexer___init__(int argc, TurboObject** args);
TurboObject* t_impl_Lexer_tokenize(int argc, TurboObject** args);
TurboObject* t_impl_Lexer_tokenize_line(int argc, TurboObject** args);
TurboObject* t_Parser = NULL;
TurboObject* t_impl_Parser___init__(int argc, TurboObject** args);
TurboObject* t_impl_Parser_peek(int argc, TurboObject** args);
TurboObject* t_impl_Parser_consume(int argc, TurboObject** args);
TurboObject* t_impl_Parser_match(int argc, TurboObject** args);
TurboObject* t_impl_Parser_parse_expr(int argc, TurboObject** args);
TurboObject* t_impl_Parser_parse_and(int argc, TurboObject** args);
TurboObject* t_impl_Parser_parse_not(int argc, TurboObject** args);
TurboObject* t_impl_Parser_parse_comparison(int argc, TurboObject** args);
TurboObject* t_impl_Parser_parse_sum(int argc, TurboObject** args);
TurboObject* t_impl_Parser_parse_term(int argc, TurboObject** args);
TurboObject* t_impl_Parser_parse_factor(int argc, TurboObject** args);
TurboObject* t_impl_Parser_parse_primary(int argc, TurboObject** args);
TurboObject* t_impl_Parser_parse_atom(int argc, TurboObject** args);
TurboObject* t_impl_Parser_parse_suite(int argc, TurboObject** args);
TurboObject* t_impl_Parser_parse_stmt(int argc, TurboObject** args);
TurboObject* t_impl_Parser_parse_module(int argc, TurboObject** args);
TurboObject* t_impl_collect_locals(int argc, TurboObject** args);
TurboObject* t_collect_locals = NULL;
TurboObject* t_impl_collect_globals(int argc, TurboObject** args);
TurboObject* t_collect_globals = NULL;
TurboObject* t_impl_escape_c_string(int argc, TurboObject** args);
TurboObject* t_escape_c_string = NULL;
TurboObject* t_CodeGen = NULL;
TurboObject* t_impl_CodeGen___init__(int argc, TurboObject** args);
TurboObject* t_impl_CodeGen_write_header(int argc, TurboObject** args);
TurboObject* t_impl_CodeGen_write_func(int argc, TurboObject** args);
TurboObject* t_impl_CodeGen_write_main(int argc, TurboObject** args);
TurboObject* t_impl_CodeGen_write_code(int argc, TurboObject** args);
TurboObject* t_impl_CodeGen_gen_expr(int argc, TurboObject** args);
TurboObject* t_impl_CodeGen_gen_func_def(int argc, TurboObject** args);
TurboObject* t_impl_CodeGen_gen_class_def(int argc, TurboObject** args);
TurboObject* t_impl_CodeGen_gen_block_stmts(int argc, TurboObject** args);
TurboObject* t_impl_CodeGen_gen_stmt(int argc, TurboObject** args);
TurboObject* t_impl_main(int argc, TurboObject** args);
TurboObject* t_main = NULL;

TurboObject* t_impl_Token___init__(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_type = (argc > 1) ? args[1] : turbo_none;
    TurboObject* t_value = (argc > 2) ? args[2] : turbo_none;
    TurboObject* t_line = (argc > 3) ? args[3] : turbo_none;
    turbo_setattr(t_self, "type", t_type);
    turbo_setattr(t_self, "value", t_value);
    turbo_setattr(t_self, "line", t_line);
    return turbo_none;
}

TurboObject* t_impl_ASTNode___init__(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_type = (argc > 1) ? args[1] : turbo_none;
    TurboObject* t_value = (argc > 2) ? args[2] : turbo_none;
    TurboObject* t_children = (argc > 3) ? args[3] : turbo_none;
    turbo_setattr(t_self, "type", t_type);
    turbo_setattr(t_self, "value", t_value);
    turbo_setattr(t_self, "children", t_children);
    return turbo_none;
}

TurboObject* t_impl_count_leading_spaces(int argc, TurboObject** args) {
    TurboObject* t_line = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_count = turbo_none;
    TurboObject* t_c = turbo_none;
    t_count = make_int(0);
    while (turbo_is_truthy(turbo_lt(t_count, turbo_call(t_len, 1, (TurboObject*[]){t_line})))) {
        t_c = turbo_getitem(t_line, t_count);
        if (turbo_is_truthy(turbo_eq(t_c, make_str(" ")))) {
            t_count = turbo_add(t_count, make_int(1));
        } else {
            if (turbo_is_truthy(turbo_eq(t_c, make_str("\t")))) {
                t_count = turbo_add(t_count, make_int(4));
            } else {
                break;
            }
        }
    }
    return t_count;
    return turbo_none;
}

TurboObject* t_impl_is_empty_or_comment(int argc, TurboObject** args) {
    TurboObject* t_line = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_i = turbo_none;
    TurboObject* t_c = turbo_none;
    t_i = make_int(0);
    while (turbo_is_truthy(turbo_lt(t_i, turbo_call(t_len, 1, (TurboObject*[]){t_line})))) {
        t_c = turbo_getitem(t_line, t_i);
        if (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = turbo_eq(t_c, make_str(" ")); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("\t")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("\r")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("\n")); }))) {
            t_i = turbo_add(t_i, make_int(1));
        } else {
            if (turbo_is_truthy(turbo_eq(t_c, make_str("#")))) {
                return turbo_true;
            } else {
                return turbo_false;
            }
        }
    }
    return turbo_true;
    return turbo_none;
}

TurboObject* t_impl_Lexer___init__(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_text = (argc > 1) ? args[1] : turbo_none;
    turbo_setattr(t_self, "text", t_text);
    turbo_setattr(t_self, "indent_stack", ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, make_int(0));  _lst; }));
    turbo_setattr(t_self, "tokens", make_list());
    return turbo_none;
}

TurboObject* t_impl_Lexer_tokenize(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_lines = turbo_none;
    TurboObject* t_line_idx = turbo_none;
    TurboObject* t_line = turbo_none;
    TurboObject* t_line_num = turbo_none;
    TurboObject* t_spaces = turbo_none;
    TurboObject* t_current_indent = turbo_none;
    t_lines = turbo_call_method(turbo_getattr(t_self, "text"), "split", 1, (TurboObject*[]){make_str("\n")});
    t_line_idx = make_int(0);
    while (turbo_is_truthy(turbo_lt(t_line_idx, turbo_call(t_len, 1, (TurboObject*[]){t_lines})))) {
        t_line = turbo_getitem(t_lines, t_line_idx);
        t_line_num = turbo_add(t_line_idx, make_int(1));
        t_line_idx = turbo_add(t_line_idx, make_int(1));
        if (turbo_is_truthy(turbo_gt(turbo_call(t_len, 1, (TurboObject*[]){t_line}), make_int(0)))) {
            if (turbo_is_truthy(turbo_eq(turbo_getitem(t_line, turbo_sub(turbo_call(t_len, 1, (TurboObject*[]){t_line}), make_int(1))), make_str("\r")))) {
                t_line = turbo_slice(t_line, make_int(0), turbo_sub(turbo_call(t_len, 1, (TurboObject*[]){t_line}), make_int(1)));
            }
        }
        if (turbo_is_truthy(turbo_call(t_is_empty_or_comment, 1, (TurboObject*[]){t_line}))) {
            continue;
        }
        t_spaces = turbo_call(t_count_leading_spaces, 1, (TurboObject*[]){t_line});
        t_current_indent = turbo_getitem(turbo_getattr(t_self, "indent_stack"), turbo_sub(turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_self, "indent_stack")}), make_int(1)));
        if (turbo_is_truthy(turbo_gt(t_spaces, t_current_indent))) {
            turbo_call_method(turbo_getattr(t_self, "indent_stack"), "append", 1, (TurboObject*[]){t_spaces});
            turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){make_str("INDENT"), make_str(""), t_line_num})});
        } else {
            if (turbo_is_truthy(turbo_lt(t_spaces, t_current_indent))) {
                while (turbo_is_truthy(({ TurboObject* _lh = turbo_gt(turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_self, "indent_stack")}), make_int(0)); turbo_is_truthy(_lh) ? turbo_gt(turbo_getitem(turbo_getattr(t_self, "indent_stack"), turbo_sub(turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_self, "indent_stack")}), make_int(1))), t_spaces) : _lh; }))) {
                    turbo_call_method(turbo_getattr(t_self, "indent_stack"), "pop", 0, NULL);
                    turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){make_str("DEDENT"), make_str(""), t_line_num})});
                }
                if (turbo_is_truthy(turbo_ne(turbo_getitem(turbo_getattr(t_self, "indent_stack"), turbo_sub(turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_self, "indent_stack")}), make_int(1))), t_spaces))) {
                    turbo_call(t_print, 1, (TurboObject*[]){turbo_add(make_str("IndentationError at line "), turbo_call(t_str, 1, (TurboObject*[]){t_line_num}))});
                }
            }
        }
        turbo_call_method(t_self, "tokenize_line", 2, (TurboObject*[]){t_line, t_line_num});
        turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){make_str("NEWLINE"), make_str(""), t_line_num})});
    }
    while (turbo_is_truthy(turbo_gt(turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_self, "indent_stack")}), make_int(1)))) {
        turbo_call_method(turbo_getattr(t_self, "indent_stack"), "pop", 0, NULL);
        turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){make_str("DEDENT"), make_str(""), t_line_idx})});
    }
    turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){make_str("EOF"), make_str(""), t_line_idx})});
    return turbo_none;
}

TurboObject* t_impl_Lexer_tokenize_line(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_line = (argc > 1) ? args[1] : turbo_none;
    TurboObject* t_line_num = (argc > 2) ? args[2] : turbo_none;
    TurboObject* t_p = turbo_none;
    TurboObject* t_c = turbo_none;
    TurboObject* t_two_chars = turbo_none;
    TurboObject* t_quote = turbo_none;
    TurboObject* t_val = turbo_none;
    TurboObject* t_ec = turbo_none;
    TurboObject* t_char = turbo_none;
    TurboObject* t_is_keyword = turbo_none;
    TurboObject* t_keywords = turbo_none;
    TurboObject* t_k_idx = turbo_none;
    t_p = make_int(0);
    while (turbo_is_truthy(turbo_lt(t_p, turbo_call(t_len, 1, (TurboObject*[]){t_line})))) {
        t_c = turbo_getitem(t_line, t_p);
        if (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = turbo_eq(t_c, make_str(" ")); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("\t")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("\r")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("\n")); }))) {
            t_p = turbo_add(t_p, make_int(1));
            continue;
        }
        if (turbo_is_truthy(turbo_eq(t_c, make_str("#")))) {
            break;
        }
        if (turbo_is_truthy(turbo_lt(turbo_add(t_p, make_int(1)), turbo_call(t_len, 1, (TurboObject*[]){t_line})))) {
            t_two_chars = turbo_add(turbo_getitem(t_line, t_p), turbo_getitem(t_line, turbo_add(t_p, make_int(1))));
            if (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = turbo_eq(t_two_chars, make_str("==")); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str("!=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str("<=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str(">=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str("+=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str("-=")); }))) {
                turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){t_two_chars, t_two_chars, t_line_num})});
                t_p = turbo_add(t_p, make_int(2));
                continue;
            }
        }
        if (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = turbo_eq(t_c, make_str("+")); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("-")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("*")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("/")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("%")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("<")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str(">")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("(")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str(")")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("[")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("]")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("{")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("}")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str(":")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str(",")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str(".")); }))) {
            turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){t_c, t_c, t_line_num})});
            t_p = turbo_add(t_p, make_int(1));
            continue;
        }
        if (turbo_is_truthy(({ TurboObject* _lh = turbo_eq(t_c, make_str("\"")); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("'")); }))) {
            t_quote = t_c;
            t_val = make_str("");
            t_p = turbo_add(t_p, make_int(1));
            while (turbo_is_truthy(({ TurboObject* _lh = turbo_lt(t_p, turbo_call(t_len, 1, (TurboObject*[]){t_line})); turbo_is_truthy(_lh) ? turbo_ne(turbo_getitem(t_line, t_p), t_quote) : _lh; }))) {
                if (turbo_is_truthy(turbo_eq(turbo_getitem(t_line, t_p), make_str("\\")))) {
                    t_p = turbo_add(t_p, make_int(1));
                    if (turbo_is_truthy(turbo_lt(t_p, turbo_call(t_len, 1, (TurboObject*[]){t_line})))) {
                        t_ec = turbo_getitem(t_line, t_p);
                        if (turbo_is_truthy(turbo_eq(t_ec, make_str("n")))) {
                            t_val = turbo_add(t_val, make_str("\n"));
                        } else {
                            if (turbo_is_truthy(turbo_eq(t_ec, make_str("t")))) {
                                t_val = turbo_add(t_val, make_str("\t"));
                            } else {
                                if (turbo_is_truthy(turbo_eq(t_ec, make_str("r")))) {
                                    t_val = turbo_add(t_val, make_str("\r"));
                                } else {
                                    t_val = turbo_add(t_val, t_ec);
                                }
                            }
                        }
                    }
                    t_p = turbo_add(t_p, make_int(1));
                } else {
                    t_val = turbo_add(t_val, turbo_getitem(t_line, t_p));
                    t_p = turbo_add(t_p, make_int(1));
                }
            }
            t_p = turbo_add(t_p, make_int(1));
            turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){make_str("STRING"), t_val, t_line_num})});
            continue;
        }
        if (turbo_is_truthy(({ TurboObject* _lh = turbo_gte(t_c, make_str("0")); turbo_is_truthy(_lh) ? turbo_lte(t_c, make_str("9")) : _lh; }))) {
            t_val = make_str("");
            while (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = turbo_lt(t_p, turbo_call(t_len, 1, (TurboObject*[]){t_line})); turbo_is_truthy(_lh) ? turbo_gte(turbo_getitem(t_line, t_p), make_str("0")) : _lh; }); turbo_is_truthy(_lh) ? turbo_lte(turbo_getitem(t_line, t_p), make_str("9")) : _lh; }))) {
                t_val = turbo_add(t_val, turbo_getitem(t_line, t_p));
                t_p = turbo_add(t_p, make_int(1));
            }
            turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){make_str("NUMBER"), t_val, t_line_num})});
            continue;
        }
        if (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = turbo_gte(t_c, make_str("a")); turbo_is_truthy(_lh) ? turbo_lte(t_c, make_str("z")) : _lh; }); turbo_is_truthy(_lh) ? _lh : ({ TurboObject* _lh = turbo_gte(t_c, make_str("A")); turbo_is_truthy(_lh) ? turbo_lte(t_c, make_str("Z")) : _lh; }); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("_")); }))) {
            t_val = make_str("");
            while (turbo_is_truthy(turbo_lt(t_p, turbo_call(t_len, 1, (TurboObject*[]){t_line})))) {
                t_char = turbo_getitem(t_line, t_p);
                if (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = turbo_gte(t_char, make_str("a")); turbo_is_truthy(_lh) ? turbo_lte(t_char, make_str("z")) : _lh; }); turbo_is_truthy(_lh) ? _lh : ({ TurboObject* _lh = turbo_gte(t_char, make_str("A")); turbo_is_truthy(_lh) ? turbo_lte(t_char, make_str("Z")) : _lh; }); }); turbo_is_truthy(_lh) ? _lh : ({ TurboObject* _lh = turbo_gte(t_char, make_str("0")); turbo_is_truthy(_lh) ? turbo_lte(t_char, make_str("9")) : _lh; }); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_char, make_str("_")); }))) {
                    t_val = turbo_add(t_val, t_char);
                    t_p = turbo_add(t_p, make_int(1));
                } else {
                    break;
                }
            }
            t_is_keyword = turbo_false;
            t_keywords = ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, make_str("def")); turbo_list_append(_lst, make_str("class")); turbo_list_append(_lst, make_str("if")); turbo_list_append(_lst, make_str("elif")); turbo_list_append(_lst, make_str("else")); turbo_list_append(_lst, make_str("while")); turbo_list_append(_lst, make_str("for")); turbo_list_append(_lst, make_str("in")); turbo_list_append(_lst, make_str("return")); turbo_list_append(_lst, make_str("and")); turbo_list_append(_lst, make_str("or")); turbo_list_append(_lst, make_str("not")); turbo_list_append(_lst, make_str("pass")); turbo_list_append(_lst, make_str("None")); turbo_list_append(_lst, make_str("True")); turbo_list_append(_lst, make_str("False")); turbo_list_append(_lst, make_str("import")); turbo_list_append(_lst, make_str("break")); turbo_list_append(_lst, make_str("continue"));  _lst; });
            t_k_idx = make_int(0);
            while (turbo_is_truthy(turbo_lt(t_k_idx, turbo_call(t_len, 1, (TurboObject*[]){t_keywords})))) {
                if (turbo_is_truthy(turbo_eq(turbo_getitem(t_keywords, t_k_idx), t_val))) {
                    t_is_keyword = turbo_true;
                    break;
                }
                t_k_idx = turbo_add(t_k_idx, make_int(1));
            }
            if (turbo_is_truthy(t_is_keyword)) {
                turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){t_val, t_val, t_line_num})});
            } else {
                turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){make_str("NAME"), t_val, t_line_num})});
            }
            continue;
        }
        turbo_call(t_print, 1, (TurboObject*[]){turbo_add(turbo_add(turbo_add(make_str("Lexer error: unexpected character "), t_c), make_str(" at line ")), turbo_call(t_str, 1, (TurboObject*[]){t_line_num}))});
        t_p = turbo_add(t_p, make_int(1));
    }
    return turbo_none;
}

TurboObject* t_impl_Parser___init__(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_tokens = (argc > 1) ? args[1] : turbo_none;
    turbo_setattr(t_self, "tokens", t_tokens);
    turbo_setattr(t_self, "pos", make_int(0));
    return turbo_none;
}

TurboObject* t_impl_Parser_peek(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    if (turbo_is_truthy(turbo_gte(turbo_getattr(t_self, "pos"), turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_self, "tokens")})))) {
        return turbo_call(t_Token, 3, (TurboObject*[]){make_str("EOF"), make_str(""), turbo_sub(make_int(0), make_int(1))});
    }
    return turbo_getitem(turbo_getattr(t_self, "tokens"), turbo_getattr(t_self, "pos"));
    return turbo_none;
}

TurboObject* t_impl_Parser_consume(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_expected_type = (argc > 1) ? args[1] : turbo_none;
    TurboObject* t_t = turbo_none;
    t_t = turbo_call_method(t_self, "peek", 0, NULL);
    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), t_expected_type))) {
        turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
        return t_t;
    }
    turbo_call(t_print, 1, (TurboObject*[]){turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(make_str("Parser error: expected "), t_expected_type), make_str(" but got ")), turbo_getattr(t_t, "type")), make_str(" at line ")), turbo_call(t_str, 1, (TurboObject*[]){turbo_getattr(t_t, "line")}))});
    turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
    return t_t;
    return turbo_none;
}

TurboObject* t_impl_Parser_match(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_token_type = (argc > 1) ? args[1] : turbo_none;
    TurboObject* t_t = turbo_none;
    t_t = turbo_call_method(t_self, "peek", 0, NULL);
    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), t_token_type))) {
        turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
        return turbo_true;
    }
    return turbo_false;
    return turbo_none;
}

TurboObject* t_impl_Parser_parse_expr(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_node = turbo_none;
    TurboObject* t_right = turbo_none;
    t_node = turbo_call_method(t_self, "parse_and", 0, NULL);
    while (turbo_is_truthy(turbo_call_method(t_self, "match", 1, (TurboObject*[]){make_str("or")}))) {
        t_right = turbo_call_method(t_self, "parse_and", 0, NULL);
        t_node = turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("OR"), make_str(""), ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_node); turbo_list_append(_lst, t_right);  _lst; })});
    }
    return t_node;
    return turbo_none;
}

TurboObject* t_impl_Parser_parse_and(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_node = turbo_none;
    TurboObject* t_right = turbo_none;
    t_node = turbo_call_method(t_self, "parse_not", 0, NULL);
    while (turbo_is_truthy(turbo_call_method(t_self, "match", 1, (TurboObject*[]){make_str("and")}))) {
        t_right = turbo_call_method(t_self, "parse_not", 0, NULL);
        t_node = turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("AND"), make_str(""), ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_node); turbo_list_append(_lst, t_right);  _lst; })});
    }
    return t_node;
    return turbo_none;
}

TurboObject* t_impl_Parser_parse_not(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_expr = turbo_none;
    if (turbo_is_truthy(turbo_call_method(t_self, "match", 1, (TurboObject*[]){make_str("not")}))) {
        t_expr = turbo_call_method(t_self, "parse_not", 0, NULL);
        return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("NOT"), make_str(""), ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_expr);  _lst; })});
    }
    return turbo_call_method(t_self, "parse_comparison", 0, NULL);
    return turbo_none;
}

TurboObject* t_impl_Parser_parse_comparison(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_node = turbo_none;
    TurboObject* t_t = turbo_none;
    TurboObject* t_right = turbo_none;
    t_node = turbo_call_method(t_self, "parse_sum", 0, NULL);
    t_t = turbo_call_method(t_self, "peek", 0, NULL);
    if (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = turbo_eq(turbo_getattr(t_t, "type"), make_str("==")); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getattr(t_t, "type"), make_str("!=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getattr(t_t, "type"), make_str("<")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getattr(t_t, "type"), make_str(">")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getattr(t_t, "type"), make_str("<=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getattr(t_t, "type"), make_str(">=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getattr(t_t, "type"), make_str("in")); }))) {
        turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
        t_right = turbo_call_method(t_self, "parse_sum", 0, NULL);
        t_node = turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("COMP"), turbo_getattr(t_t, "type"), ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_node); turbo_list_append(_lst, t_right);  _lst; })});
    }
    return t_node;
    return turbo_none;
}

TurboObject* t_impl_Parser_parse_sum(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_node = turbo_none;
    TurboObject* t_t = turbo_none;
    TurboObject* t_right = turbo_none;
    t_node = turbo_call_method(t_self, "parse_term", 0, NULL);
    while (turbo_is_truthy(turbo_true)) {
        t_t = turbo_call_method(t_self, "peek", 0, NULL);
        if (turbo_is_truthy(({ TurboObject* _lh = turbo_eq(turbo_getattr(t_t, "type"), make_str("+")); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getattr(t_t, "type"), make_str("-")); }))) {
            turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
            t_right = turbo_call_method(t_self, "parse_term", 0, NULL);
            t_node = turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("BINOP"), turbo_getattr(t_t, "type"), ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_node); turbo_list_append(_lst, t_right);  _lst; })});
        } else {
            break;
        }
    }
    return t_node;
    return turbo_none;
}

TurboObject* t_impl_Parser_parse_term(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_node = turbo_none;
    TurboObject* t_t = turbo_none;
    TurboObject* t_right = turbo_none;
    t_node = turbo_call_method(t_self, "parse_factor", 0, NULL);
    while (turbo_is_truthy(turbo_true)) {
        t_t = turbo_call_method(t_self, "peek", 0, NULL);
        if (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = turbo_eq(turbo_getattr(t_t, "type"), make_str("*")); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getattr(t_t, "type"), make_str("/")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getattr(t_t, "type"), make_str("%")); }))) {
            turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
            t_right = turbo_call_method(t_self, "parse_factor", 0, NULL);
            t_node = turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("BINOP"), turbo_getattr(t_t, "type"), ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_node); turbo_list_append(_lst, t_right);  _lst; })});
        } else {
            break;
        }
    }
    return t_node;
    return turbo_none;
}

TurboObject* t_impl_Parser_parse_factor(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_t = turbo_none;
    TurboObject* t_factor = turbo_none;
    t_t = turbo_call_method(t_self, "peek", 0, NULL);
    if (turbo_is_truthy(({ TurboObject* _lh = turbo_eq(turbo_getattr(t_t, "type"), make_str("+")); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getattr(t_t, "type"), make_str("-")); }))) {
        turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
        t_factor = turbo_call_method(t_self, "parse_factor", 0, NULL);
        return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("UNARY"), turbo_getattr(t_t, "type"), ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_factor);  _lst; })});
    }
    return turbo_call_method(t_self, "parse_primary", 0, NULL);
    return turbo_none;
}

TurboObject* t_impl_Parser_parse_primary(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_node = turbo_none;
    TurboObject* t_t = turbo_none;
    TurboObject* t_args = turbo_none;
    TurboObject* t_attr = turbo_none;
    TurboObject* t_is_slice = turbo_none;
    TurboObject* t_start = turbo_none;
    TurboObject* t_end = turbo_none;
    TurboObject* t_first = turbo_none;
    t_node = turbo_call_method(t_self, "parse_atom", 0, NULL);
    while (turbo_is_truthy(turbo_true)) {
        t_t = turbo_call_method(t_self, "peek", 0, NULL);
        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("(")))) {
            turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
            t_args = make_list();
            if (turbo_is_truthy(turbo_ne(turbo_getattr(turbo_call_method(t_self, "peek", 0, NULL), "type"), make_str(")")))) {
                turbo_call_method(t_args, "append", 1, (TurboObject*[]){turbo_call_method(t_self, "parse_expr", 0, NULL)});
                while (turbo_is_truthy(turbo_call_method(t_self, "match", 1, (TurboObject*[]){make_str(",")}))) {
                    turbo_call_method(t_args, "append", 1, (TurboObject*[]){turbo_call_method(t_self, "parse_expr", 0, NULL)});
                }
            }
            turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str(")")});
            t_node = turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("CALL"), make_str(""), ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_node); turbo_list_append(_lst, turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("LIST"), make_str(""), t_args}));  _lst; })});
        } else {
            if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str(".")))) {
                turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                t_attr = turbo_getattr(turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("NAME")}), "value");
                t_node = turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("ATTR"), t_attr, ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_node);  _lst; })});
            } else {
                if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("[")))) {
                    turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                    t_is_slice = turbo_false;
                    t_start = turbo_none;
                    t_end = turbo_none;
                    if (turbo_is_truthy(turbo_call_method(t_self, "match", 1, (TurboObject*[]){make_str(":")}))) {
                        t_is_slice = turbo_true;
                        t_start = turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("CONST_NONE"), make_str(""), make_list()});
                        if (turbo_is_truthy(turbo_ne(turbo_getattr(turbo_call_method(t_self, "peek", 0, NULL), "type"), make_str("]")))) {
                            t_end = turbo_call_method(t_self, "parse_expr", 0, NULL);
                        } else {
                            t_end = turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("CONST_NONE"), make_str(""), make_list()});
                        }
                    } else {
                        t_first = turbo_call_method(t_self, "parse_expr", 0, NULL);
                        if (turbo_is_truthy(turbo_call_method(t_self, "match", 1, (TurboObject*[]){make_str(":")}))) {
                            t_is_slice = turbo_true;
                            t_start = t_first;
                            if (turbo_is_truthy(turbo_ne(turbo_getattr(turbo_call_method(t_self, "peek", 0, NULL), "type"), make_str("]")))) {
                                t_end = turbo_call_method(t_self, "parse_expr", 0, NULL);
                            } else {
                                t_end = turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("CONST_NONE"), make_str(""), make_list()});
                            }
                        } else {
                            t_node = turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("SUBSCRIPT"), make_str(""), ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_node); turbo_list_append(_lst, t_first);  _lst; })});
                        }
                    }
                    if (turbo_is_truthy(t_is_slice)) {
                        t_node = turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("SLICE"), make_str(""), ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_node); turbo_list_append(_lst, t_start); turbo_list_append(_lst, t_end);  _lst; })});
                    }
                    turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("]")});
                } else {
                    break;
                }
            }
        }
    }
    return t_node;
    return turbo_none;
}

TurboObject* t_impl_Parser_parse_atom(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_t = turbo_none;
    TurboObject* t_node = turbo_none;
    TurboObject* t_elements = turbo_none;
    TurboObject* t_keys = turbo_none;
    TurboObject* t_values = turbo_none;
    TurboObject* t_key = turbo_none;
    TurboObject* t_val = turbo_none;
    t_t = turbo_call_method(t_self, "peek", 0, NULL);
    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("NUMBER")))) {
        turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
        return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("CONST_INT"), turbo_getattr(t_t, "value"), make_list()});
    } else {
        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("STRING")))) {
            turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
            return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("CONST_STR"), turbo_getattr(t_t, "value"), make_list()});
        } else {
            if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("None")))) {
                turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("CONST_NONE"), make_str(""), make_list()});
            } else {
                if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("True")))) {
                    turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                    return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("CONST_BOOL"), make_str("True"), make_list()});
                } else {
                    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("False")))) {
                        turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                        return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("CONST_BOOL"), make_str("False"), make_list()});
                    } else {
                        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("NAME")))) {
                            turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                            return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("NAME"), turbo_getattr(t_t, "value"), make_list()});
                        } else {
                            if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("(")))) {
                                turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                                t_node = turbo_call_method(t_self, "parse_expr", 0, NULL);
                                turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str(")")});
                                return t_node;
                            } else {
                                if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("[")))) {
                                    turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                                    t_elements = make_list();
                                    if (turbo_is_truthy(turbo_ne(turbo_getattr(turbo_call_method(t_self, "peek", 0, NULL), "type"), make_str("]")))) {
                                        turbo_call_method(t_elements, "append", 1, (TurboObject*[]){turbo_call_method(t_self, "parse_expr", 0, NULL)});
                                        while (turbo_is_truthy(turbo_call_method(t_self, "match", 1, (TurboObject*[]){make_str(",")}))) {
                                            turbo_call_method(t_elements, "append", 1, (TurboObject*[]){turbo_call_method(t_self, "parse_expr", 0, NULL)});
                                        }
                                    }
                                    turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("]")});
                                    return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("LIST"), make_str(""), t_elements});
                                } else {
                                    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("{")))) {
                                        turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                                        t_keys = make_list();
                                        t_values = make_list();
                                        if (turbo_is_truthy(turbo_ne(turbo_getattr(turbo_call_method(t_self, "peek", 0, NULL), "type"), make_str("}")))) {
                                            t_key = turbo_call_method(t_self, "parse_expr", 0, NULL);
                                            turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str(":")});
                                            t_val = turbo_call_method(t_self, "parse_expr", 0, NULL);
                                            turbo_call_method(t_keys, "append", 1, (TurboObject*[]){t_key});
                                            turbo_call_method(t_values, "append", 1, (TurboObject*[]){t_val});
                                            while (turbo_is_truthy(turbo_call_method(t_self, "match", 1, (TurboObject*[]){make_str(",")}))) {
                                                t_key = turbo_call_method(t_self, "parse_expr", 0, NULL);
                                                turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str(":")});
                                                t_val = turbo_call_method(t_self, "parse_expr", 0, NULL);
                                                turbo_call_method(t_keys, "append", 1, (TurboObject*[]){t_key});
                                                turbo_call_method(t_values, "append", 1, (TurboObject*[]){t_val});
                                            }
                                        }
                                        turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("}")});
                                        return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("DICT"), make_str(""), ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("LIST"), make_str(""), t_keys})); turbo_list_append(_lst, turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("LIST"), make_str(""), t_values}));  _lst; })});
                                    } else {
                                        turbo_call(t_print, 1, (TurboObject*[]){turbo_add(turbo_add(turbo_add(make_str("Parser error: unexpected token in expression "), turbo_getattr(t_t, "type")), make_str(" at line ")), turbo_call(t_str, 1, (TurboObject*[]){turbo_getattr(t_t, "line")}))});
                                        turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                                        return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("CONST_NONE"), make_str(""), make_list()});
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return turbo_none;
}

TurboObject* t_impl_Parser_parse_suite(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_stmts = turbo_none;
    turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str(":")});
    turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("NEWLINE")});
    turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("INDENT")});
    t_stmts = make_list();
    while (turbo_is_truthy(({ TurboObject* _lh = turbo_ne(turbo_getattr(turbo_call_method(t_self, "peek", 0, NULL), "type"), make_str("DEDENT")); turbo_is_truthy(_lh) ? turbo_ne(turbo_getattr(turbo_call_method(t_self, "peek", 0, NULL), "type"), make_str("EOF")) : _lh; }))) {
        turbo_call_method(t_stmts, "append", 1, (TurboObject*[]){turbo_call_method(t_self, "parse_stmt", 0, NULL)});
    }
    turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("DEDENT")});
    return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("BLOCK"), make_str(""), t_stmts});
    return turbo_none;
}

TurboObject* t_impl_Parser_parse_stmt(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_t = turbo_none;
    TurboObject* t_name = turbo_none;
    TurboObject* t_params = turbo_none;
    TurboObject* t_body = turbo_none;
    TurboObject* t_params_node = turbo_none;
    TurboObject* t_p_idx = turbo_none;
    TurboObject* t_base_name = turbo_none;
    TurboObject* t_cond = turbo_none;
    TurboObject* t_then_branch = turbo_none;
    TurboObject* t_curr_node = turbo_none;
    TurboObject* t_parent_node = turbo_none;
    TurboObject* t_next_t = turbo_none;
    TurboObject* t_elif_cond = turbo_none;
    TurboObject* t_elif_body = turbo_none;
    TurboObject* t_elif_node = turbo_none;
    TurboObject* t_else_body = turbo_none;
    TurboObject* t_var_name = turbo_none;
    TurboObject* t_iterable = turbo_none;
    TurboObject* t_value = turbo_none;
    TurboObject* t_mod_name = turbo_none;
    TurboObject* t_expr = turbo_none;
    TurboObject* t_op = turbo_none;
    t_t = turbo_call_method(t_self, "peek", 0, NULL);
    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("def")))) {
        turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
        t_name = turbo_getattr(turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("NAME")}), "value");
        turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("(")});
        t_params = make_list();
        if (turbo_is_truthy(turbo_ne(turbo_getattr(turbo_call_method(t_self, "peek", 0, NULL), "type"), make_str(")")))) {
            turbo_call_method(t_params, "append", 1, (TurboObject*[]){turbo_getattr(turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("NAME")}), "value")});
            while (turbo_is_truthy(turbo_call_method(t_self, "match", 1, (TurboObject*[]){make_str(",")}))) {
                turbo_call_method(t_params, "append", 1, (TurboObject*[]){turbo_getattr(turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("NAME")}), "value")});
            }
        }
        turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str(")")});
        t_body = turbo_call_method(t_self, "parse_suite", 0, NULL);
        t_params_node = turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("PARAMS"), make_str(""), make_list()});
        t_p_idx = make_int(0);
        while (turbo_is_truthy(turbo_lt(t_p_idx, turbo_call(t_len, 1, (TurboObject*[]){t_params})))) {
            turbo_call_method(turbo_getattr(t_params_node, "children"), "append", 1, (TurboObject*[]){turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("NAME"), turbo_getitem(t_params, t_p_idx), make_list()})});
            t_p_idx = turbo_add(t_p_idx, make_int(1));
        }
        return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("DEF"), t_name, ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_params_node); turbo_list_append(_lst, t_body);  _lst; })});
    } else {
        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("class")))) {
            turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
            t_name = turbo_getattr(turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("NAME")}), "value");
            if (turbo_is_truthy(turbo_call_method(t_self, "match", 1, (TurboObject*[]){make_str("(")}))) {
                t_base_name = turbo_getattr(turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("NAME")}), "value");
                turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str(")")});
            }
            t_body = turbo_call_method(t_self, "parse_suite", 0, NULL);
            return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("CLASS"), t_name, ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_body);  _lst; })});
        } else {
            if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("if")))) {
                turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                t_cond = turbo_call_method(t_self, "parse_expr", 0, NULL);
                t_then_branch = turbo_call_method(t_self, "parse_suite", 0, NULL);
                t_curr_node = turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("IF"), make_str(""), ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_cond); turbo_list_append(_lst, t_then_branch);  _lst; })});
                t_parent_node = t_curr_node;
                while (turbo_is_truthy(turbo_true)) {
                    t_next_t = turbo_call_method(t_self, "peek", 0, NULL);
                    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_next_t, "type"), make_str("elif")))) {
                        turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                        t_elif_cond = turbo_call_method(t_self, "parse_expr", 0, NULL);
                        t_elif_body = turbo_call_method(t_self, "parse_suite", 0, NULL);
                        t_elif_node = turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("IF"), make_str(""), ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_elif_cond); turbo_list_append(_lst, t_elif_body);  _lst; })});
                        turbo_call_method(turbo_getattr(t_parent_node, "children"), "append", 1, (TurboObject*[]){t_elif_node});
                        t_parent_node = t_elif_node;
                    } else {
                        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_next_t, "type"), make_str("else")))) {
                            turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                            t_else_body = turbo_call_method(t_self, "parse_suite", 0, NULL);
                            turbo_call_method(turbo_getattr(t_parent_node, "children"), "append", 1, (TurboObject*[]){t_else_body});
                            break;
                        } else {
                            break;
                        }
                    }
                }
                return t_curr_node;
            } else {
                if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("while")))) {
                    turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                    t_cond = turbo_call_method(t_self, "parse_expr", 0, NULL);
                    t_body = turbo_call_method(t_self, "parse_suite", 0, NULL);
                    return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("WHILE"), make_str(""), ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_cond); turbo_list_append(_lst, t_body);  _lst; })});
                } else {
                    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("for")))) {
                        turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                        t_var_name = turbo_getattr(turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("NAME")}), "value");
                        turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("in")});
                        t_iterable = turbo_call_method(t_self, "parse_expr", 0, NULL);
                        t_body = turbo_call_method(t_self, "parse_suite", 0, NULL);
                        return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("FOR"), t_var_name, ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_iterable); turbo_list_append(_lst, t_body);  _lst; })});
                    } else {
                        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("return")))) {
                            turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                            t_value = turbo_none;
                            if (turbo_is_truthy(turbo_ne(turbo_getattr(turbo_call_method(t_self, "peek", 0, NULL), "type"), make_str("NEWLINE")))) {
                                t_value = turbo_call_method(t_self, "parse_expr", 0, NULL);
                            } else {
                                t_value = turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("CONST_NONE"), make_str(""), make_list()});
                            }
                            turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("NEWLINE")});
                            return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("RETURN"), make_str(""), ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_value);  _lst; })});
                        } else {
                            if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("pass")))) {
                                turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                                turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("NEWLINE")});
                                return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("PASS"), make_str(""), make_list()});
                            } else {
                                if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("break")))) {
                                    turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                                    turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("NEWLINE")});
                                    return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("BREAK"), make_str(""), make_list()});
                                } else {
                                    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("continue")))) {
                                        turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                                        turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("NEWLINE")});
                                        return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("CONTINUE"), make_str(""), make_list()});
                                    } else {
                                        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_t, "type"), make_str("import")))) {
                                            turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                                            t_mod_name = turbo_getattr(turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("NAME")}), "value");
                                            turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("NEWLINE")});
                                            return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("IMPORT"), t_mod_name, make_list()});
                                        } else {
                                            t_expr = turbo_call_method(t_self, "parse_expr", 0, NULL);
                                            t_next_t = turbo_call_method(t_self, "peek", 0, NULL);
                                            if (turbo_is_truthy(turbo_eq(turbo_getattr(t_next_t, "type"), make_str("=")))) {
                                                turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                                                t_value = turbo_call_method(t_self, "parse_expr", 0, NULL);
                                                turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("NEWLINE")});
                                                return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("ASSIGN"), make_str(""), ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_expr); turbo_list_append(_lst, t_value);  _lst; })});
                                            } else {
                                                if (turbo_is_truthy(({ TurboObject* _lh = turbo_eq(turbo_getattr(t_next_t, "type"), make_str("+=")); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getattr(t_next_t, "type"), make_str("-=")); }))) {
                                                    t_op = turbo_getattr(t_next_t, "type");
                                                    turbo_setattr(t_self, "pos", turbo_add(turbo_getattr(t_self, "pos"), make_int(1)));
                                                    t_value = turbo_call_method(t_self, "parse_expr", 0, NULL);
                                                    turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("NEWLINE")});
                                                    return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("AUGASSIGN"), t_op, ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_expr); turbo_list_append(_lst, t_value);  _lst; })});
                                                } else {
                                                    turbo_call_method(t_self, "consume", 1, (TurboObject*[]){make_str("NEWLINE")});
                                                    return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("EXPR"), make_str(""), ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, t_expr);  _lst; })});
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return turbo_none;
}

TurboObject* t_impl_Parser_parse_module(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_stmts = turbo_none;
    t_stmts = make_list();
    while (turbo_is_truthy(turbo_ne(turbo_getattr(turbo_call_method(t_self, "peek", 0, NULL), "type"), make_str("EOF")))) {
        if (turbo_is_truthy(turbo_call_method(t_self, "match", 1, (TurboObject*[]){make_str("NEWLINE")}))) {
            continue;
        }
        turbo_call_method(t_stmts, "append", 1, (TurboObject*[]){turbo_call_method(t_self, "parse_stmt", 0, NULL)});
    }
    return turbo_call(t_ASTNode, 3, (TurboObject*[]){make_str("BLOCK"), make_str(""), t_stmts});
    return turbo_none;
}

TurboObject* t_impl_collect_locals(int argc, TurboObject** args) {
    TurboObject* t_node = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_locals_list = (argc > 1) ? args[1] : turbo_none;
    TurboObject* t_target = turbo_none;
    TurboObject* t_name = turbo_none;
    TurboObject* t_found = turbo_none;
    TurboObject* t_i = turbo_none;
    TurboObject* t_idx = turbo_none;
    if (turbo_is_truthy(({ TurboObject* _lh = turbo_eq(turbo_getattr(t_node, "type"), make_str("ASSIGN")); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getattr(t_node, "type"), make_str("AUGASSIGN")); }))) {
        t_target = turbo_getitem(turbo_getattr(t_node, "children"), make_int(0));
        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_target, "type"), make_str("NAME")))) {
            t_name = turbo_getattr(t_target, "value");
            t_found = turbo_false;
            t_i = make_int(0);
            while (turbo_is_truthy(turbo_lt(t_i, turbo_call(t_len, 1, (TurboObject*[]){t_locals_list})))) {
                if (turbo_is_truthy(turbo_eq(turbo_getitem(t_locals_list, t_i), t_name))) {
                    t_found = turbo_true;
                    break;
                }
                t_i = turbo_add(t_i, make_int(1));
            }
            if (turbo_is_truthy(turbo_not(t_found))) {
                turbo_call_method(t_locals_list, "append", 1, (TurboObject*[]){t_name});
            }
        }
    } else {
        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("FOR")))) {
            t_name = turbo_getattr(t_node, "value");
            t_found = turbo_false;
            t_i = make_int(0);
            while (turbo_is_truthy(turbo_lt(t_i, turbo_call(t_len, 1, (TurboObject*[]){t_locals_list})))) {
                if (turbo_is_truthy(turbo_eq(turbo_getitem(t_locals_list, t_i), t_name))) {
                    t_found = turbo_true;
                    break;
                }
                t_i = turbo_add(t_i, make_int(1));
            }
            if (turbo_is_truthy(turbo_not(t_found))) {
                turbo_call_method(t_locals_list, "append", 1, (TurboObject*[]){t_name});
            }
        }
    }
    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("DEF")))) {
        return turbo_none;
    }
    t_idx = make_int(0);
    while (turbo_is_truthy(turbo_lt(t_idx, turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_node, "children")})))) {
        turbo_call(t_collect_locals, 2, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), t_idx), t_locals_list});
        t_idx = turbo_add(t_idx, make_int(1));
    }
    return turbo_none;
}

TurboObject* t_impl_collect_globals(int argc, TurboObject** args) {
    TurboObject* t_block_node = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_globals_list = (argc > 1) ? args[1] : turbo_none;
    TurboObject* t_stmts = turbo_none;
    TurboObject* t_idx = turbo_none;
    TurboObject* t_node = turbo_none;
    TurboObject* t_target = turbo_none;
    TurboObject* t_name = turbo_none;
    TurboObject* t_found = turbo_none;
    TurboObject* t_g_idx = turbo_none;
    t_stmts = turbo_getattr(t_block_node, "children");
    t_idx = make_int(0);
    while (turbo_is_truthy(turbo_lt(t_idx, turbo_call(t_len, 1, (TurboObject*[]){t_stmts})))) {
        t_node = turbo_getitem(t_stmts, t_idx);
        if (turbo_is_truthy(({ TurboObject* _lh = turbo_eq(turbo_getattr(t_node, "type"), make_str("ASSIGN")); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getattr(t_node, "type"), make_str("AUGASSIGN")); }))) {
            t_target = turbo_getitem(turbo_getattr(t_node, "children"), make_int(0));
            if (turbo_is_truthy(turbo_eq(turbo_getattr(t_target, "type"), make_str("NAME")))) {
                t_name = turbo_getattr(t_target, "value");
                t_found = turbo_false;
                t_g_idx = make_int(0);
                while (turbo_is_truthy(turbo_lt(t_g_idx, turbo_call(t_len, 1, (TurboObject*[]){t_globals_list})))) {
                    if (turbo_is_truthy(turbo_eq(turbo_getitem(t_globals_list, t_g_idx), t_name))) {
                        t_found = turbo_true;
                        break;
                    }
                    t_g_idx = turbo_add(t_g_idx, make_int(1));
                }
                if (turbo_is_truthy(turbo_not(t_found))) {
                    turbo_call_method(t_globals_list, "append", 1, (TurboObject*[]){t_name});
                }
            }
        } else {
            if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("FOR")))) {
                t_name = turbo_getattr(t_node, "value");
                t_found = turbo_false;
                t_g_idx = make_int(0);
                while (turbo_is_truthy(turbo_lt(t_g_idx, turbo_call(t_len, 1, (TurboObject*[]){t_globals_list})))) {
                    if (turbo_is_truthy(turbo_eq(turbo_getitem(t_globals_list, t_g_idx), t_name))) {
                        t_found = turbo_true;
                        break;
                    }
                    t_g_idx = turbo_add(t_g_idx, make_int(1));
                }
                if (turbo_is_truthy(turbo_not(t_found))) {
                    turbo_call_method(t_globals_list, "append", 1, (TurboObject*[]){t_name});
                }
            }
        }
        t_idx = turbo_add(t_idx, make_int(1));
    }
    return turbo_none;
}

TurboObject* t_impl_escape_c_string(int argc, TurboObject** args) {
    TurboObject* t_s = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_res = turbo_none;
    TurboObject* t_i = turbo_none;
    TurboObject* t_c = turbo_none;
    t_res = make_str("");
    t_i = make_int(0);
    while (turbo_is_truthy(turbo_lt(t_i, turbo_call(t_len, 1, (TurboObject*[]){t_s})))) {
        t_c = turbo_getitem(t_s, t_i);
        if (turbo_is_truthy(turbo_eq(t_c, make_str("\n")))) {
            t_res = turbo_add(t_res, make_str("\\n"));
        } else {
            if (turbo_is_truthy(turbo_eq(t_c, make_str("\t")))) {
                t_res = turbo_add(t_res, make_str("\\t"));
            } else {
                if (turbo_is_truthy(turbo_eq(t_c, make_str("\r")))) {
                    t_res = turbo_add(t_res, make_str("\\r"));
                } else {
                    if (turbo_is_truthy(turbo_eq(t_c, make_str("\\")))) {
                        t_res = turbo_add(t_res, make_str("\\\\"));
                    } else {
                        if (turbo_is_truthy(turbo_eq(t_c, make_str("\"")))) {
                            t_res = turbo_add(t_res, make_str("\\\""));
                        } else {
                            t_res = turbo_add(t_res, t_c);
                        }
                    }
                }
            }
        }
        t_i = turbo_add(t_i, make_int(1));
    }
    return t_res;
    return turbo_none;
}

TurboObject* t_impl_CodeGen___init__(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    turbo_setattr(t_self, "header", make_str(""));
    turbo_setattr(t_self, "funcs", make_str(""));
    turbo_setattr(t_self, "main_body", make_str(""));
    turbo_setattr(t_self, "indent_level", make_int(0));
    turbo_setattr(t_self, "local_vars", make_list());
    return turbo_none;
}

TurboObject* t_impl_CodeGen_write_header(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_s = (argc > 1) ? args[1] : turbo_none;
    turbo_setattr(t_self, "header", turbo_add(turbo_getattr(t_self, "header"), t_s));
    return turbo_none;
}

TurboObject* t_impl_CodeGen_write_func(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_s = (argc > 1) ? args[1] : turbo_none;
    turbo_setattr(t_self, "funcs", turbo_add(turbo_getattr(t_self, "funcs"), t_s));
    return turbo_none;
}

TurboObject* t_impl_CodeGen_write_main(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_s = (argc > 1) ? args[1] : turbo_none;
    TurboObject* t_i = turbo_none;
    t_i = make_int(0);
    while (turbo_is_truthy(turbo_lt(t_i, turbo_getattr(t_self, "indent_level")))) {
        turbo_setattr(t_self, "main_body", turbo_add(turbo_getattr(t_self, "main_body"), make_str("    ")));
        t_i = turbo_add(t_i, make_int(1));
    }
    turbo_setattr(t_self, "main_body", turbo_add(turbo_add(turbo_getattr(t_self, "main_body"), t_s), make_str("\n")));
    return turbo_none;
}

TurboObject* t_impl_CodeGen_write_code(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_s = (argc > 1) ? args[1] : turbo_none;
    TurboObject* t_is_in_func = (argc > 2) ? args[2] : turbo_none;
    TurboObject* t_i = turbo_none;
    if (turbo_is_truthy(t_is_in_func)) {
        t_i = make_int(0);
        while (turbo_is_truthy(turbo_lt(t_i, turbo_getattr(t_self, "indent_level")))) {
            turbo_setattr(t_self, "funcs", turbo_add(turbo_getattr(t_self, "funcs"), make_str("    ")));
            t_i = turbo_add(t_i, make_int(1));
        }
        turbo_setattr(t_self, "funcs", turbo_add(turbo_add(turbo_getattr(t_self, "funcs"), t_s), make_str("\n")));
    } else {
        turbo_call_method(t_self, "write_main", 1, (TurboObject*[]){t_s});
    }
    return turbo_none;
}

TurboObject* t_impl_CodeGen_gen_expr(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_node = (argc > 1) ? args[1] : turbo_none;
    TurboObject* t_left = turbo_none;
    TurboObject* t_right = turbo_none;
    TurboObject* t_op = turbo_none;
    TurboObject* t_operand = turbo_none;
    TurboObject* t_func_node = turbo_none;
    TurboObject* t_args_list = turbo_none;
    TurboObject* t_argc = turbo_none;
    TurboObject* t_args_c = turbo_none;
    TurboObject* t_a_idx = turbo_none;
    TurboObject* t_obj_expr = turbo_none;
    TurboObject* t_method_name = turbo_none;
    TurboObject* t_func_expr = turbo_none;
    TurboObject* t_attr_name = turbo_none;
    TurboObject* t_idx_expr = turbo_none;
    TurboObject* t_start_expr = turbo_none;
    TurboObject* t_end_expr = turbo_none;
    TurboObject* t_c_code = turbo_none;
    TurboObject* t_keys = turbo_none;
    TurboObject* t_values = turbo_none;
    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("CONST_INT")))) {
        return turbo_add(turbo_add(make_str("make_int("), turbo_getattr(t_node, "value")), make_str(")"));
    } else {
        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("CONST_STR")))) {
            return turbo_add(turbo_add(make_str("make_str(\""), turbo_call(t_escape_c_string, 1, (TurboObject*[]){turbo_getattr(t_node, "value")})), make_str("\")"));
        } else {
            if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("CONST_NONE")))) {
                return make_str("turbo_none");
            } else {
                if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("CONST_BOOL")))) {
                    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "value"), make_str("True")))) {
                        return make_str("turbo_true");
                    } else {
                        return make_str("turbo_false");
                    }
                } else {
                    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("NAME")))) {
                        return turbo_add(make_str("t_"), turbo_getattr(t_node, "value"));
                    } else {
                        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("BINOP")))) {
                            t_left = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(0))});
                            t_right = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(1))});
                            t_op = turbo_getattr(t_node, "value");
                            if (turbo_is_truthy(turbo_eq(t_op, make_str("+")))) {
                                return turbo_add(turbo_add(turbo_add(turbo_add(make_str("turbo_add("), t_left), make_str(", ")), t_right), make_str(")"));
                            } else {
                                if (turbo_is_truthy(turbo_eq(t_op, make_str("-")))) {
                                    return turbo_add(turbo_add(turbo_add(turbo_add(make_str("turbo_sub("), t_left), make_str(", ")), t_right), make_str(")"));
                                } else {
                                    if (turbo_is_truthy(turbo_eq(t_op, make_str("*")))) {
                                        return turbo_add(turbo_add(turbo_add(turbo_add(make_str("turbo_mul("), t_left), make_str(", ")), t_right), make_str(")"));
                                    } else {
                                        if (turbo_is_truthy(turbo_eq(t_op, make_str("/")))) {
                                            return turbo_add(turbo_add(turbo_add(turbo_add(make_str("turbo_div("), t_left), make_str(", ")), t_right), make_str(")"));
                                        } else {
                                            if (turbo_is_truthy(turbo_eq(t_op, make_str("%")))) {
                                                return turbo_add(turbo_add(turbo_add(turbo_add(make_str("turbo_mod("), t_left), make_str(", ")), t_right), make_str(")"));
                                            }
                                        }
                                    }
                                }
                            }
                        } else {
                            if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("UNARY")))) {
                                t_operand = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(0))});
                                t_op = turbo_getattr(t_node, "value");
                                if (turbo_is_truthy(turbo_eq(t_op, make_str("-")))) {
                                    return turbo_add(turbo_add(make_str("turbo_sub(make_int(0), "), t_operand), make_str(")"));
                                } else {
                                    if (turbo_is_truthy(turbo_eq(t_op, make_str("+")))) {
                                        return t_operand;
                                    }
                                }
                            } else {
                                if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("COMP")))) {
                                    t_left = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(0))});
                                    t_right = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(1))});
                                    t_op = turbo_getattr(t_node, "value");
                                    if (turbo_is_truthy(turbo_eq(t_op, make_str("==")))) {
                                        return turbo_add(turbo_add(turbo_add(turbo_add(make_str("turbo_eq("), t_left), make_str(", ")), t_right), make_str(")"));
                                    } else {
                                        if (turbo_is_truthy(turbo_eq(t_op, make_str("!=")))) {
                                            return turbo_add(turbo_add(turbo_add(turbo_add(make_str("turbo_ne("), t_left), make_str(", ")), t_right), make_str(")"));
                                        } else {
                                            if (turbo_is_truthy(turbo_eq(t_op, make_str("<")))) {
                                                return turbo_add(turbo_add(turbo_add(turbo_add(make_str("turbo_lt("), t_left), make_str(", ")), t_right), make_str(")"));
                                            } else {
                                                if (turbo_is_truthy(turbo_eq(t_op, make_str(">")))) {
                                                    return turbo_add(turbo_add(turbo_add(turbo_add(make_str("turbo_gt("), t_left), make_str(", ")), t_right), make_str(")"));
                                                } else {
                                                    if (turbo_is_truthy(turbo_eq(t_op, make_str("<=")))) {
                                                        return turbo_add(turbo_add(turbo_add(turbo_add(make_str("turbo_lte("), t_left), make_str(", ")), t_right), make_str(")"));
                                                    } else {
                                                        if (turbo_is_truthy(turbo_eq(t_op, make_str(">=")))) {
                                                            return turbo_add(turbo_add(turbo_add(turbo_add(make_str("turbo_gte("), t_left), make_str(", ")), t_right), make_str(")"));
                                                        } else {
                                                            if (turbo_is_truthy(turbo_eq(t_op, make_str("in")))) {
                                                                return turbo_add(turbo_add(turbo_add(turbo_add(make_str("make_bool(turbo_in("), t_left), make_str(", ")), t_right), make_str("))"));
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                } else {
                                    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("NOT")))) {
                                        t_operand = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(0))});
                                        return turbo_add(turbo_add(make_str("turbo_not("), t_operand), make_str(")"));
                                    } else {
                                        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("AND")))) {
                                            t_left = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(0))});
                                            t_right = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(1))});
                                            return turbo_add(turbo_add(turbo_add(turbo_add(make_str("({ TurboObject* _lh = "), t_left), make_str("; turbo_is_truthy(_lh) ? ")), t_right), make_str(" : _lh; })"));
                                        } else {
                                            if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("OR")))) {
                                                t_left = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(0))});
                                                t_right = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(1))});
                                                return turbo_add(turbo_add(turbo_add(turbo_add(make_str("({ TurboObject* _lh = "), t_left), make_str("; turbo_is_truthy(_lh) ? _lh : ")), t_right), make_str("; })"));
                                            } else {
                                                if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("CALL")))) {
                                                    t_func_node = turbo_getitem(turbo_getattr(t_node, "children"), make_int(0));
                                                    t_args_list = turbo_getattr(turbo_getitem(turbo_getattr(t_node, "children"), make_int(1)), "children");
                                                    t_argc = turbo_call(t_len, 1, (TurboObject*[]){t_args_list});
                                                    t_args_c = make_str("NULL");
                                                    if (turbo_is_truthy(turbo_gt(t_argc, make_int(0)))) {
                                                        t_args_c = make_str("(TurboObject*[]){");
                                                        t_a_idx = make_int(0);
                                                        while (turbo_is_truthy(turbo_lt(t_a_idx, t_argc))) {
                                                            t_args_c = turbo_add(t_args_c, turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(t_args_list, t_a_idx)}));
                                                            if (turbo_is_truthy(turbo_lt(t_a_idx, turbo_sub(t_argc, make_int(1))))) {
                                                                t_args_c = turbo_add(t_args_c, make_str(", "));
                                                            }
                                                            t_a_idx = turbo_add(t_a_idx, make_int(1));
                                                        }
                                                        t_args_c = turbo_add(t_args_c, make_str("}"));
                                                    }
                                                    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_func_node, "type"), make_str("ATTR")))) {
                                                        t_obj_expr = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_func_node, "children"), make_int(0))});
                                                        t_method_name = turbo_getattr(t_func_node, "value");
                                                        return turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(make_str("turbo_call_method("), t_obj_expr), make_str(", \"")), t_method_name), make_str("\", ")), turbo_call(t_str, 1, (TurboObject*[]){t_argc})), make_str(", ")), t_args_c), make_str(")"));
                                                    } else {
                                                        t_func_expr = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){t_func_node});
                                                        return turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(make_str("turbo_call("), t_func_expr), make_str(", ")), turbo_call(t_str, 1, (TurboObject*[]){t_argc})), make_str(", ")), t_args_c), make_str(")"));
                                                    }
                                                } else {
                                                    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("ATTR")))) {
                                                        t_obj_expr = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(0))});
                                                        t_attr_name = turbo_getattr(t_node, "value");
                                                        return turbo_add(turbo_add(turbo_add(turbo_add(make_str("turbo_getattr("), t_obj_expr), make_str(", \"")), t_attr_name), make_str("\")"));
                                                    } else {
                                                        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("SUBSCRIPT")))) {
                                                            t_obj_expr = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(0))});
                                                            t_idx_expr = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(1))});
                                                            return turbo_add(turbo_add(turbo_add(turbo_add(make_str("turbo_getitem("), t_obj_expr), make_str(", ")), t_idx_expr), make_str(")"));
                                                        } else {
                                                            if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("SLICE")))) {
                                                                t_obj_expr = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(0))});
                                                                t_start_expr = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(1))});
                                                                t_end_expr = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(2))});
                                                                return turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(make_str("turbo_slice("), t_obj_expr), make_str(", ")), t_start_expr), make_str(", ")), t_end_expr), make_str(")"));
                                                            } else {
                                                                if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("LIST")))) {
                                                                    t_argc = turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_node, "children")});
                                                                    if (turbo_is_truthy(turbo_eq(t_argc, make_int(0)))) {
                                                                        return make_str("make_list()");
                                                                    }
                                                                    t_c_code = make_str("({ TurboObject* _lst = make_list(); ");
                                                                    t_a_idx = make_int(0);
                                                                    while (turbo_is_truthy(turbo_lt(t_a_idx, t_argc))) {
                                                                        t_c_code = turbo_add(turbo_add(turbo_add(t_c_code, make_str("turbo_list_append(_lst, ")), turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), t_a_idx)})), make_str("); "));
                                                                        t_a_idx = turbo_add(t_a_idx, make_int(1));
                                                                    }
                                                                    t_c_code = turbo_add(t_c_code, make_str(" _lst; })"));
                                                                    return t_c_code;
                                                                } else {
                                                                    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("DICT")))) {
                                                                        t_keys = turbo_getattr(turbo_getitem(turbo_getattr(t_node, "children"), make_int(0)), "children");
                                                                        t_values = turbo_getattr(turbo_getitem(turbo_getattr(t_node, "children"), make_int(1)), "children");
                                                                        t_argc = turbo_call(t_len, 1, (TurboObject*[]){t_keys});
                                                                        if (turbo_is_truthy(turbo_eq(t_argc, make_int(0)))) {
                                                                            return make_str("make_dict()");
                                                                        }
                                                                        t_c_code = make_str("({ TurboObject* _d = make_dict(); ");
                                                                        t_a_idx = make_int(0);
                                                                        while (turbo_is_truthy(turbo_lt(t_a_idx, t_argc))) {
                                                                            t_c_code = turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(t_c_code, make_str("turbo_setitem(_d, ")), turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(t_keys, t_a_idx)})), make_str(", ")), turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(t_values, t_a_idx)})), make_str("); "));
                                                                            t_a_idx = turbo_add(t_a_idx, make_int(1));
                                                                        }
                                                                        t_c_code = turbo_add(t_c_code, make_str(" _d; })"));
                                                                        return t_c_code;
                                                                    } else {
                                                                        turbo_call(t_print, 1, (TurboObject*[]){turbo_add(make_str("CodeGen error: unknown expression "), turbo_getattr(t_node, "type"))});
                                                                        return make_str("turbo_none");
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return turbo_none;
}

TurboObject* t_impl_CodeGen_gen_func_def(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_node = (argc > 1) ? args[1] : turbo_none;
    TurboObject* t_class_name = (argc > 2) ? args[2] : turbo_none;
    TurboObject* t_func_name = turbo_none;
    TurboObject* t_impl_name = turbo_none;
    TurboObject* t_params_node = turbo_none;
    TurboObject* t_body_node = turbo_none;
    TurboObject* t_old_locals = turbo_none;
    TurboObject* t_p_idx = turbo_none;
    TurboObject* t_p_name = turbo_none;
    TurboObject* t_l_idx = turbo_none;
    TurboObject* t_l_name = turbo_none;
    TurboObject* t_is_param = turbo_none;
    t_func_name = turbo_getattr(t_node, "value");
    if (turbo_is_truthy(turbo_ne(t_class_name, make_str("")))) {
        t_impl_name = turbo_add(turbo_add(turbo_add(make_str("t_impl_"), t_class_name), make_str("_")), t_func_name);
    } else {
        t_impl_name = turbo_add(make_str("t_impl_"), t_func_name);
    }
    t_params_node = turbo_getitem(turbo_getattr(t_node, "children"), make_int(0));
    t_body_node = turbo_getitem(turbo_getattr(t_node, "children"), make_int(1));
    t_old_locals = turbo_getattr(t_self, "local_vars");
    turbo_setattr(t_self, "local_vars", make_list());
    t_p_idx = make_int(0);
    while (turbo_is_truthy(turbo_lt(t_p_idx, turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_params_node, "children")})))) {
        turbo_call_method(turbo_getattr(t_self, "local_vars"), "append", 1, (TurboObject*[]){turbo_getattr(turbo_getitem(turbo_getattr(t_params_node, "children"), t_p_idx), "value")});
        t_p_idx = turbo_add(t_p_idx, make_int(1));
    }
    turbo_call(t_collect_locals, 2, (TurboObject*[]){t_body_node, turbo_getattr(t_self, "local_vars")});
    turbo_call_method(t_self, "write_header", 1, (TurboObject*[]){turbo_add(turbo_add(make_str("TurboObject* "), t_impl_name), make_str("(int argc, TurboObject** args);\n"))});
    turbo_setattr(t_self, "funcs", turbo_add(turbo_add(turbo_add(turbo_getattr(t_self, "funcs"), make_str("TurboObject* ")), t_impl_name), make_str("(int argc, TurboObject** args) {\n")));
    t_p_idx = make_int(0);
    while (turbo_is_truthy(turbo_lt(t_p_idx, turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_params_node, "children")})))) {
        t_p_name = turbo_getattr(turbo_getitem(turbo_getattr(t_params_node, "children"), t_p_idx), "value");
        turbo_setattr(t_self, "funcs", turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(turbo_getattr(t_self, "funcs"), make_str("    TurboObject* t_")), t_p_name), make_str(" = (argc > ")), turbo_call(t_str, 1, (TurboObject*[]){t_p_idx})), make_str(") ? args[")), turbo_call(t_str, 1, (TurboObject*[]){t_p_idx})), make_str("] : turbo_none;\n")));
        t_p_idx = turbo_add(t_p_idx, make_int(1));
    }
    t_l_idx = make_int(0);
    while (turbo_is_truthy(turbo_lt(t_l_idx, turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_self, "local_vars")})))) {
        t_l_name = turbo_getitem(turbo_getattr(t_self, "local_vars"), t_l_idx);
        t_is_param = turbo_false;
        t_p_idx = make_int(0);
        while (turbo_is_truthy(turbo_lt(t_p_idx, turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_params_node, "children")})))) {
            if (turbo_is_truthy(turbo_eq(turbo_getattr(turbo_getitem(turbo_getattr(t_params_node, "children"), t_p_idx), "value"), t_l_name))) {
                t_is_param = turbo_true;
                break;
            }
            t_p_idx = turbo_add(t_p_idx, make_int(1));
        }
        if (turbo_is_truthy(turbo_not(t_is_param))) {
            turbo_setattr(t_self, "funcs", turbo_add(turbo_add(turbo_add(turbo_getattr(t_self, "funcs"), make_str("    TurboObject* t_")), t_l_name), make_str(" = turbo_none;\n")));
        }
        t_l_idx = turbo_add(t_l_idx, make_int(1));
    }
    turbo_setattr(t_self, "indent_level", make_int(1));
    turbo_call_method(t_self, "gen_block_stmts", 2, (TurboObject*[]){t_body_node, turbo_true});
    turbo_setattr(t_self, "funcs", turbo_add(turbo_getattr(t_self, "funcs"), make_str("    return turbo_none;\n")));
    turbo_setattr(t_self, "funcs", turbo_add(turbo_getattr(t_self, "funcs"), make_str("}\n\n")));
    turbo_setattr(t_self, "local_vars", t_old_locals);
    if (turbo_is_truthy(turbo_eq(t_class_name, make_str("")))) {
        turbo_call_method(t_self, "write_header", 1, (TurboObject*[]){turbo_add(turbo_add(make_str("TurboObject* t_"), t_func_name), make_str(" = NULL;\n"))});
        turbo_call_method(t_self, "write_main", 1, (TurboObject*[]){turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(make_str("t_"), t_func_name), make_str(" = make_func(t_impl_")), t_func_name), make_str(", \"")), t_func_name), make_str("\");"))});
    }
    return turbo_none;
}

TurboObject* t_impl_CodeGen_gen_class_def(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_node = (argc > 1) ? args[1] : turbo_none;
    TurboObject* t_class_name = turbo_none;
    TurboObject* t_suite = turbo_none;
    TurboObject* t_m_idx = turbo_none;
    TurboObject* t_m_node = turbo_none;
    TurboObject* t_method_name = turbo_none;
    t_class_name = turbo_getattr(t_node, "value");
    turbo_call_method(t_self, "write_header", 1, (TurboObject*[]){turbo_add(turbo_add(make_str("TurboObject* t_"), t_class_name), make_str(" = NULL;\n"))});
    turbo_call_method(t_self, "write_main", 1, (TurboObject*[]){turbo_add(turbo_add(turbo_add(turbo_add(make_str("t_"), t_class_name), make_str(" = make_class(\"")), t_class_name), make_str("\");"))});
    t_suite = turbo_getitem(turbo_getattr(t_node, "children"), make_int(0));
    t_m_idx = make_int(0);
    while (turbo_is_truthy(turbo_lt(t_m_idx, turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_suite, "children")})))) {
        t_m_node = turbo_getitem(turbo_getattr(t_suite, "children"), t_m_idx);
        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_m_node, "type"), make_str("DEF")))) {
            turbo_call_method(t_self, "gen_func_def", 2, (TurboObject*[]){t_m_node, t_class_name});
            t_method_name = turbo_getattr(t_m_node, "value");
            turbo_call_method(t_self, "write_main", 1, (TurboObject*[]){turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(make_str("turbo_class_add_method(t_"), t_class_name), make_str(", \"")), t_method_name), make_str("\", t_impl_")), t_class_name), make_str("_")), t_method_name), make_str(");"))});
        }
        t_m_idx = turbo_add(t_m_idx, make_int(1));
    }
    return turbo_none;
}

TurboObject* t_impl_CodeGen_gen_block_stmts(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_block_node = (argc > 1) ? args[1] : turbo_none;
    TurboObject* t_is_in_func = (argc > 2) ? args[2] : turbo_none;
    TurboObject* t_stmts = turbo_none;
    TurboObject* t_s_idx = turbo_none;
    t_stmts = turbo_getattr(t_block_node, "children");
    t_s_idx = make_int(0);
    while (turbo_is_truthy(turbo_lt(t_s_idx, turbo_call(t_len, 1, (TurboObject*[]){t_stmts})))) {
        turbo_call_method(t_self, "gen_stmt", 2, (TurboObject*[]){turbo_getitem(t_stmts, t_s_idx), t_is_in_func});
        t_s_idx = turbo_add(t_s_idx, make_int(1));
    }
    return turbo_none;
}

TurboObject* t_impl_CodeGen_gen_stmt(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_node = (argc > 1) ? args[1] : turbo_none;
    TurboObject* t_is_in_func = (argc > 2) ? args[2] : turbo_none;
    TurboObject* t_expr_c = turbo_none;
    TurboObject* t_target = turbo_none;
    TurboObject* t_val_node = turbo_none;
    TurboObject* t_is_sys_argv_setup = turbo_none;
    TurboObject* t_val_c = turbo_none;
    TurboObject* t_obj_c = turbo_none;
    TurboObject* t_idx_c = turbo_none;
    TurboObject* t_op = turbo_none;
    TurboObject* t_cond_c = turbo_none;
    TurboObject* t_var_name = turbo_none;
    TurboObject* t_iter_c = turbo_none;
    TurboObject* t_else_node = turbo_none;
    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("PASS")))) {
        turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){make_str("/* pass */;"), t_is_in_func});
    } else {
        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("IMPORT")))) {
            turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){turbo_add(turbo_add(make_str("/* import "), turbo_getattr(t_node, "value")), make_str(" */;")), t_is_in_func});
        } else {
            if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("BREAK")))) {
                turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){make_str("break;"), t_is_in_func});
            } else {
                if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("CONTINUE")))) {
                    turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){make_str("continue;"), t_is_in_func});
                } else {
                    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("EXPR")))) {
                        t_expr_c = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(0))});
                        turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){turbo_add(t_expr_c, make_str(";")), t_is_in_func});
                    } else {
                        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("ASSIGN")))) {
                            t_target = turbo_getitem(turbo_getattr(t_node, "children"), make_int(0));
                            t_val_node = turbo_getitem(turbo_getattr(t_node, "children"), make_int(1));
                            t_is_sys_argv_setup = turbo_false;
                            if (turbo_is_truthy(({ TurboObject* _lh = turbo_eq(turbo_getattr(t_target, "type"), make_str("NAME")); turbo_is_truthy(_lh) ? turbo_eq(turbo_getattr(t_target, "value"), make_str("sys_argv")) : _lh; }))) {
                                if (turbo_is_truthy(({ TurboObject* _lh = turbo_eq(turbo_getattr(t_val_node, "type"), make_str("ATTR")); turbo_is_truthy(_lh) ? turbo_eq(turbo_getattr(t_val_node, "value"), make_str("argv")) : _lh; }))) {
                                    if (turbo_is_truthy(({ TurboObject* _lh = turbo_eq(turbo_getattr(turbo_getitem(turbo_getattr(t_val_node, "children"), make_int(0)), "type"), make_str("NAME")); turbo_is_truthy(_lh) ? turbo_eq(turbo_getattr(turbo_getitem(turbo_getattr(t_val_node, "children"), make_int(0)), "value"), make_str("sys")) : _lh; }))) {
                                        t_is_sys_argv_setup = turbo_true;
                                    }
                                }
                            }
                            if (turbo_is_truthy(t_is_sys_argv_setup)) {
                                turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){make_str("/* ignored sys_argv = sys.argv */;"), t_is_in_func});
                            } else {
                                t_val_c = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){t_val_node});
                                if (turbo_is_truthy(turbo_eq(turbo_getattr(t_target, "type"), make_str("NAME")))) {
                                    turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){turbo_add(turbo_add(turbo_add(turbo_add(make_str("t_"), turbo_getattr(t_target, "value")), make_str(" = ")), t_val_c), make_str(";")), t_is_in_func});
                                } else {
                                    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_target, "type"), make_str("ATTR")))) {
                                        t_obj_c = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_target, "children"), make_int(0))});
                                        turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(make_str("turbo_setattr("), t_obj_c), make_str(", \"")), turbo_getattr(t_target, "value")), make_str("\", ")), t_val_c), make_str(");")), t_is_in_func});
                                    } else {
                                        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_target, "type"), make_str("SUBSCRIPT")))) {
                                            t_obj_c = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_target, "children"), make_int(0))});
                                            t_idx_c = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_target, "children"), make_int(1))});
                                            turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(make_str("turbo_setitem("), t_obj_c), make_str(", ")), t_idx_c), make_str(", ")), t_val_c), make_str(");")), t_is_in_func});
                                        }
                                    }
                                }
                            }
                        } else {
                            if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("AUGASSIGN")))) {
                                t_target = turbo_getitem(turbo_getattr(t_node, "children"), make_int(0));
                                t_val_c = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(1))});
                                t_op = turbo_getattr(t_node, "value");
                                if (turbo_is_truthy(turbo_eq(turbo_getattr(t_target, "type"), make_str("NAME")))) {
                                    if (turbo_is_truthy(turbo_eq(t_op, make_str("+=")))) {
                                        turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(make_str("t_"), turbo_getattr(t_target, "value")), make_str(" = turbo_add(t_")), turbo_getattr(t_target, "value")), make_str(", ")), t_val_c), make_str(");")), t_is_in_func});
                                    } else {
                                        if (turbo_is_truthy(turbo_eq(t_op, make_str("-=")))) {
                                            turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(turbo_add(make_str("t_"), turbo_getattr(t_target, "value")), make_str(" = turbo_sub(t_")), turbo_getattr(t_target, "value")), make_str(", ")), t_val_c), make_str(");")), t_is_in_func});
                                        }
                                    }
                                }
                            } else {
                                if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("RETURN")))) {
                                    t_val_c = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(0))});
                                    turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){turbo_add(turbo_add(make_str("return "), t_val_c), make_str(";")), t_is_in_func});
                                } else {
                                    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("WHILE")))) {
                                        t_cond_c = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(0))});
                                        turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){turbo_add(turbo_add(make_str("while (turbo_is_truthy("), t_cond_c), make_str(")) {")), t_is_in_func});
                                        turbo_setattr(t_self, "indent_level", turbo_add(turbo_getattr(t_self, "indent_level"), make_int(1)));
                                        turbo_call_method(t_self, "gen_block_stmts", 2, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(1)), t_is_in_func});
                                        turbo_setattr(t_self, "indent_level", turbo_sub(turbo_getattr(t_self, "indent_level"), make_int(1)));
                                        turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){make_str("}"), t_is_in_func});
                                    } else {
                                        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("FOR")))) {
                                            t_var_name = turbo_getattr(t_node, "value");
                                            t_iter_c = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(0))});
                                            turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){make_str("{"), t_is_in_func});
                                            turbo_setattr(t_self, "indent_level", turbo_add(turbo_getattr(t_self, "indent_level"), make_int(1)));
                                            turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){turbo_add(turbo_add(make_str("TurboObject* _iter = "), t_iter_c), make_str(";")), t_is_in_func});
                                            turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){make_str("if (_iter->type == TYPE_LIST) {"), t_is_in_func});
                                            turbo_setattr(t_self, "indent_level", turbo_add(turbo_getattr(t_self, "indent_level"), make_int(1)));
                                            turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){make_str("for (int _i = 0; _i < _iter->list_val.length; _i++) {"), t_is_in_func});
                                            turbo_setattr(t_self, "indent_level", turbo_add(turbo_getattr(t_self, "indent_level"), make_int(1)));
                                            turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){turbo_add(turbo_add(make_str("t_"), t_var_name), make_str(" = _iter->list_val.items[_i];")), t_is_in_func});
                                            turbo_call_method(t_self, "gen_block_stmts", 2, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(1)), t_is_in_func});
                                            turbo_setattr(t_self, "indent_level", turbo_sub(turbo_getattr(t_self, "indent_level"), make_int(1)));
                                            turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){make_str("}"), t_is_in_func});
                                            turbo_setattr(t_self, "indent_level", turbo_sub(turbo_getattr(t_self, "indent_level"), make_int(1)));
                                            turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){make_str("} else if (_iter->type == TYPE_STR) {"), t_is_in_func});
                                            turbo_setattr(t_self, "indent_level", turbo_add(turbo_getattr(t_self, "indent_level"), make_int(1)));
                                            turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){make_str("for (int _i = 0; _i < _iter->str_val.length; _i++) {"), t_is_in_func});
                                            turbo_setattr(t_self, "indent_level", turbo_add(turbo_getattr(t_self, "indent_level"), make_int(1)));
                                            turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){make_str("char _tmp[2] = {_iter->str_val.chars[_i], '\\0'};"), t_is_in_func});
                                            turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){turbo_add(turbo_add(make_str("t_"), t_var_name), make_str(" = make_str(_tmp);")), t_is_in_func});
                                            turbo_call_method(t_self, "gen_block_stmts", 2, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(1)), t_is_in_func});
                                            turbo_setattr(t_self, "indent_level", turbo_sub(turbo_getattr(t_self, "indent_level"), make_int(1)));
                                            turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){make_str("}"), t_is_in_func});
                                            turbo_setattr(t_self, "indent_level", turbo_sub(turbo_getattr(t_self, "indent_level"), make_int(1)));
                                            turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){make_str("}"), t_is_in_func});
                                            turbo_setattr(t_self, "indent_level", turbo_sub(turbo_getattr(t_self, "indent_level"), make_int(1)));
                                            turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){make_str("}"), t_is_in_func});
                                        } else {
                                            if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("IF")))) {
                                                t_cond_c = turbo_call_method(t_self, "gen_expr", 1, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(0))});
                                                turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){turbo_add(turbo_add(make_str("if (turbo_is_truthy("), t_cond_c), make_str(")) {")), t_is_in_func});
                                                turbo_setattr(t_self, "indent_level", turbo_add(turbo_getattr(t_self, "indent_level"), make_int(1)));
                                                turbo_call_method(t_self, "gen_block_stmts", 2, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), make_int(1)), t_is_in_func});
                                                turbo_setattr(t_self, "indent_level", turbo_sub(turbo_getattr(t_self, "indent_level"), make_int(1)));
                                                if (turbo_is_truthy(turbo_gte(turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_node, "children")}), make_int(3)))) {
                                                    t_else_node = turbo_getitem(turbo_getattr(t_node, "children"), make_int(2));
                                                    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_else_node, "type"), make_str("IF")))) {
                                                        turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){make_str("} else {"), t_is_in_func});
                                                        turbo_setattr(t_self, "indent_level", turbo_add(turbo_getattr(t_self, "indent_level"), make_int(1)));
                                                        turbo_call_method(t_self, "gen_stmt", 2, (TurboObject*[]){t_else_node, t_is_in_func});
                                                        turbo_setattr(t_self, "indent_level", turbo_sub(turbo_getattr(t_self, "indent_level"), make_int(1)));
                                                    } else {
                                                        turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){make_str("} else {"), t_is_in_func});
                                                        turbo_setattr(t_self, "indent_level", turbo_add(turbo_getattr(t_self, "indent_level"), make_int(1)));
                                                        turbo_call_method(t_self, "gen_block_stmts", 2, (TurboObject*[]){t_else_node, t_is_in_func});
                                                        turbo_setattr(t_self, "indent_level", turbo_sub(turbo_getattr(t_self, "indent_level"), make_int(1)));
                                                    }
                                                }
                                                turbo_call_method(t_self, "write_code", 2, (TurboObject*[]){make_str("}"), t_is_in_func});
                                            } else {
                                                if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("DEF")))) {
                                                    turbo_call_method(t_self, "gen_func_def", 2, (TurboObject*[]){t_node, make_str("")});
                                                } else {
                                                    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("CLASS")))) {
                                                        turbo_call_method(t_self, "gen_class_def", 1, (TurboObject*[]){t_node});
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return turbo_none;
}

TurboObject* t_impl_main(int argc, TurboObject** args) {
    TurboObject* t_input_file = turbo_none;
    TurboObject* t_output_file = turbo_none;
    TurboObject* t_i = turbo_none;
    TurboObject* t_f_in = turbo_none;
    TurboObject* t_text = turbo_none;
    TurboObject* t_lexer = turbo_none;
    TurboObject* t_parser = turbo_none;
    TurboObject* t_module_ast = turbo_none;
    TurboObject* t_cg = turbo_none;
    TurboObject* t_globals_list = turbo_none;
    TurboObject* t_g_idx = turbo_none;
    TurboObject* t_builtins = turbo_none;
    TurboObject* t_g_name = turbo_none;
    TurboObject* t_is_builtin = turbo_none;
    TurboObject* t_b_idx = turbo_none;
    TurboObject* t_c_source = turbo_none;
    TurboObject* t_f_out = turbo_none;
    if (turbo_is_truthy(turbo_lt(turbo_call(t_len, 1, (TurboObject*[]){t_sys_argv}), make_int(4)))) {
        turbo_call(t_print, 1, (TurboObject*[]){make_str("Usage: turbo <input_file> -o <output_file>")});
        return turbo_none;
    }
    t_input_file = turbo_getitem(t_sys_argv, make_int(1));
    t_output_file = make_str("");
    t_i = make_int(2);
    while (turbo_is_truthy(turbo_lt(t_i, turbo_call(t_len, 1, (TurboObject*[]){t_sys_argv})))) {
        if (turbo_is_truthy(turbo_eq(turbo_getitem(t_sys_argv, t_i), make_str("-o")))) {
            if (turbo_is_truthy(turbo_lt(turbo_add(t_i, make_int(1)), turbo_call(t_len, 1, (TurboObject*[]){t_sys_argv})))) {
                t_output_file = turbo_getitem(t_sys_argv, turbo_add(t_i, make_int(1)));
                break;
            }
        }
        t_i = turbo_add(t_i, make_int(1));
    }
    if (turbo_is_truthy(turbo_eq(t_output_file, make_str("")))) {
        turbo_call(t_print, 1, (TurboObject*[]){make_str("Error: output file not specified with -o")});
        return turbo_none;
    }
    t_f_in = turbo_call(t_open, 2, (TurboObject*[]){t_input_file, make_str("r")});
    t_text = turbo_call_method(t_f_in, "read", 0, NULL);
    turbo_call_method(t_f_in, "close", 0, NULL);
    t_lexer = turbo_call(t_Lexer, 1, (TurboObject*[]){t_text});
    turbo_call_method(t_lexer, "tokenize", 0, NULL);
    t_parser = turbo_call(t_Parser, 1, (TurboObject*[]){turbo_getattr(t_lexer, "tokens")});
    t_module_ast = turbo_call_method(t_parser, "parse_module", 0, NULL);
    t_cg = turbo_call(t_CodeGen, 0, NULL);
    t_globals_list = make_list();
    turbo_call(t_collect_globals, 2, (TurboObject*[]){t_module_ast, t_globals_list});
    t_g_idx = make_int(0);
    t_builtins = ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, make_str("print")); turbo_list_append(_lst, make_str("len")); turbo_list_append(_lst, make_str("str")); turbo_list_append(_lst, make_str("int")); turbo_list_append(_lst, make_str("ord")); turbo_list_append(_lst, make_str("chr")); turbo_list_append(_lst, make_str("range")); turbo_list_append(_lst, make_str("open")); turbo_list_append(_lst, make_str("sys_argv"));  _lst; });
    while (turbo_is_truthy(turbo_lt(t_g_idx, turbo_call(t_len, 1, (TurboObject*[]){t_globals_list})))) {
        t_g_name = turbo_getitem(t_globals_list, t_g_idx);
        t_is_builtin = turbo_false;
        t_b_idx = make_int(0);
        while (turbo_is_truthy(turbo_lt(t_b_idx, turbo_call(t_len, 1, (TurboObject*[]){t_builtins})))) {
            if (turbo_is_truthy(turbo_eq(turbo_getitem(t_builtins, t_b_idx), t_g_name))) {
                t_is_builtin = turbo_true;
                break;
            }
            t_b_idx = turbo_add(t_b_idx, make_int(1));
        }
        if (turbo_is_truthy(turbo_not(t_is_builtin))) {
            turbo_call_method(t_cg, "write_header", 1, (TurboObject*[]){turbo_add(turbo_add(make_str("TurboObject* t_"), t_g_name), make_str(" = NULL;\n"))});
        }
        t_g_idx = turbo_add(t_g_idx, make_int(1));
    }
    turbo_setattr(t_cg, "indent_level", make_int(1));
    turbo_call_method(t_cg, "gen_block_stmts", 2, (TurboObject*[]){t_module_ast, turbo_false});
    t_c_source = make_str("#include \"runtime.h\"\n#include <stdlib.h>\n\n");
    t_c_source = turbo_add(turbo_add(t_c_source, turbo_getattr(t_cg, "header")), make_str("\n"));
    t_c_source = turbo_add(turbo_add(t_c_source, turbo_getattr(t_cg, "funcs")), make_str("\n"));
    t_c_source = turbo_add(t_c_source, make_str("void turbo_main(void) {\n"));
    t_c_source = turbo_add(t_c_source, turbo_getattr(t_cg, "main_body"));
    t_c_source = turbo_add(t_c_source, make_str("}\n\n"));
    t_c_source = turbo_add(t_c_source, make_str("int main(int argc, char** argv) {\n"));
    t_c_source = turbo_add(t_c_source, make_str("    turbo_init();\n"));
    t_c_source = turbo_add(t_c_source, make_str("    t_sys_argv = make_list();\n"));
    t_c_source = turbo_add(t_c_source, make_str("    for (int i = 0; i < argc; i++) {\n"));
    t_c_source = turbo_add(t_c_source, make_str("        turbo_list_append(t_sys_argv, make_str(argv[i]));\n"));
    t_c_source = turbo_add(t_c_source, make_str("    }\n"));
    t_c_source = turbo_add(t_c_source, make_str("    turbo_main();\n"));
    t_c_source = turbo_add(t_c_source, make_str("    return 0;\n"));
    t_c_source = turbo_add(t_c_source, make_str("}\n"));
    t_f_out = turbo_call(t_open, 2, (TurboObject*[]){t_output_file, make_str("w")});
    turbo_call_method(t_f_out, "write", 1, (TurboObject*[]){t_c_source});
    turbo_call_method(t_f_out, "close", 0, NULL);
    return turbo_none;
}


void turbo_main(void) {
    /* import sys */;
    /* ignored sys_argv = sys.argv */;
    t_Token = make_class("Token");
    turbo_class_add_method(t_Token, "__init__", t_impl_Token___init__);
    t_ASTNode = make_class("ASTNode");
    turbo_class_add_method(t_ASTNode, "__init__", t_impl_ASTNode___init__);
    t_count_leading_spaces = make_func(t_impl_count_leading_spaces, "count_leading_spaces");
    t_is_empty_or_comment = make_func(t_impl_is_empty_or_comment, "is_empty_or_comment");
    t_Lexer = make_class("Lexer");
    turbo_class_add_method(t_Lexer, "__init__", t_impl_Lexer___init__);
    turbo_class_add_method(t_Lexer, "tokenize", t_impl_Lexer_tokenize);
    turbo_class_add_method(t_Lexer, "tokenize_line", t_impl_Lexer_tokenize_line);
    t_Parser = make_class("Parser");
    turbo_class_add_method(t_Parser, "__init__", t_impl_Parser___init__);
    turbo_class_add_method(t_Parser, "peek", t_impl_Parser_peek);
    turbo_class_add_method(t_Parser, "consume", t_impl_Parser_consume);
    turbo_class_add_method(t_Parser, "match", t_impl_Parser_match);
    turbo_class_add_method(t_Parser, "parse_expr", t_impl_Parser_parse_expr);
    turbo_class_add_method(t_Parser, "parse_and", t_impl_Parser_parse_and);
    turbo_class_add_method(t_Parser, "parse_not", t_impl_Parser_parse_not);
    turbo_class_add_method(t_Parser, "parse_comparison", t_impl_Parser_parse_comparison);
    turbo_class_add_method(t_Parser, "parse_sum", t_impl_Parser_parse_sum);
    turbo_class_add_method(t_Parser, "parse_term", t_impl_Parser_parse_term);
    turbo_class_add_method(t_Parser, "parse_factor", t_impl_Parser_parse_factor);
    turbo_class_add_method(t_Parser, "parse_primary", t_impl_Parser_parse_primary);
    turbo_class_add_method(t_Parser, "parse_atom", t_impl_Parser_parse_atom);
    turbo_class_add_method(t_Parser, "parse_suite", t_impl_Parser_parse_suite);
    turbo_class_add_method(t_Parser, "parse_stmt", t_impl_Parser_parse_stmt);
    turbo_class_add_method(t_Parser, "parse_module", t_impl_Parser_parse_module);
    t_collect_locals = make_func(t_impl_collect_locals, "collect_locals");
    t_collect_globals = make_func(t_impl_collect_globals, "collect_globals");
    t_escape_c_string = make_func(t_impl_escape_c_string, "escape_c_string");
    t_CodeGen = make_class("CodeGen");
    turbo_class_add_method(t_CodeGen, "__init__", t_impl_CodeGen___init__);
    turbo_class_add_method(t_CodeGen, "write_header", t_impl_CodeGen_write_header);
    turbo_class_add_method(t_CodeGen, "write_func", t_impl_CodeGen_write_func);
    turbo_class_add_method(t_CodeGen, "write_main", t_impl_CodeGen_write_main);
    turbo_class_add_method(t_CodeGen, "write_code", t_impl_CodeGen_write_code);
    turbo_class_add_method(t_CodeGen, "gen_expr", t_impl_CodeGen_gen_expr);
    turbo_class_add_method(t_CodeGen, "gen_func_def", t_impl_CodeGen_gen_func_def);
    turbo_class_add_method(t_CodeGen, "gen_class_def", t_impl_CodeGen_gen_class_def);
    turbo_class_add_method(t_CodeGen, "gen_block_stmts", t_impl_CodeGen_gen_block_stmts);
    turbo_class_add_method(t_CodeGen, "gen_stmt", t_impl_CodeGen_gen_stmt);
    t_main = make_func(t_impl_main, "main");
    turbo_call(t_main, 0, NULL);
}

int main(int argc, char** argv) {
    turbo_init();
    t_sys_argv = make_list();
    for (int i = 0; i < argc; i++) {
        turbo_list_append(t_sys_argv, make_str(argv[i]));
    }
    turbo_main();
    return 0;
}
