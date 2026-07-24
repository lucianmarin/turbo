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
TurboObject* t_impl_Lexer_collect_locals(int argc, TurboObject** args);
TurboObject* t_impl_Lexer_collect_globals(int argc, TurboObject** args);
TurboObject* t_impl_Lexer_scan_global_nonlocal_names(int argc, TurboObject** args);
TurboObject* t_impl_Lexer_escape_c_string(int argc, TurboObject** args);
TurboObject* t_impl_Lexer_main(int argc, TurboObject** args);

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
    t_count = make_int("0");
    while (turbo_is_truthy(turbo_lt(t_count, turbo_call(t_len, 1, (TurboObject*[]){t_line})))) {
        t_c = turbo_getitem(t_line, t_count);
        if (turbo_is_truthy(turbo_eq(t_c, make_str(" ")))) {
            t_count = turbo_add(t_count, make_int("1"));
        } else {
            if (turbo_is_truthy(turbo_eq(t_c, make_str("\t")))) {
                t_count = turbo_add(t_count, make_int("4"));
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
    t_i = make_int("0");
    while (turbo_is_truthy(turbo_lt(t_i, turbo_call(t_len, 1, (TurboObject*[]){t_line})))) {
        t_c = turbo_getitem(t_line, t_i);
        if (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = turbo_eq(t_c, make_str(" ")); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("\t")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("\r")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("\n")); }))) {
            t_i = turbo_add(t_i, make_int("1"));
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
    turbo_setattr(t_self, "indent_stack", ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, make_int("0"));  _lst; }));
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
    t_line_idx = make_int("0");
    while (turbo_is_truthy(turbo_lt(t_line_idx, turbo_call(t_len, 1, (TurboObject*[]){t_lines})))) {
        t_line = turbo_getitem(t_lines, t_line_idx);
        t_line_num = turbo_add(t_line_idx, make_int("1"));
        t_line_idx = turbo_add(t_line_idx, make_int("1"));
        if (turbo_is_truthy(turbo_gt(turbo_call(t_len, 1, (TurboObject*[]){t_line}), make_int("0")))) {
            if (turbo_is_truthy(turbo_eq(turbo_getitem(t_line, turbo_sub(turbo_call(t_len, 1, (TurboObject*[]){t_line}), make_int("1"))), make_str("\r")))) {
                t_line = turbo_slice(t_line, make_int("0"), turbo_sub(turbo_call(t_len, 1, (TurboObject*[]){t_line}), make_int("1")));
            }
        }
        if (turbo_is_truthy(turbo_call(t_is_empty_or_comment, 1, (TurboObject*[]){t_line}))) {
            continue;
        }
        t_spaces = turbo_call(t_count_leading_spaces, 1, (TurboObject*[]){t_line});
        t_current_indent = turbo_getitem(turbo_getattr(t_self, "indent_stack"), turbo_sub(turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_self, "indent_stack")}), make_int("1")));
        if (turbo_is_truthy(turbo_gt(t_spaces, t_current_indent))) {
            turbo_call_method(turbo_getattr(t_self, "indent_stack"), "append", 1, (TurboObject*[]){t_spaces});
            turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){make_str("INDENT"), make_str(""), t_line_num})});
        } else {
            if (turbo_is_truthy(turbo_lt(t_spaces, t_current_indent))) {
                while (turbo_is_truthy(({ TurboObject* _lh = turbo_gt(turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_self, "indent_stack")}), make_int("0")); turbo_is_truthy(_lh) ? turbo_gt(turbo_getitem(turbo_getattr(t_self, "indent_stack"), turbo_sub(turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_self, "indent_stack")}), make_int("1"))), t_spaces) : _lh; }))) {
                    turbo_call_method(turbo_getattr(t_self, "indent_stack"), "pop", 0, NULL);
                    turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){make_str("DEDENT"), make_str(""), t_line_num})});
                }
                if (turbo_is_truthy(turbo_ne(turbo_getitem(turbo_getattr(t_self, "indent_stack"), turbo_sub(turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_self, "indent_stack")}), make_int("1"))), t_spaces))) {
                    turbo_call(t_print, 1, (TurboObject*[]){turbo_add(make_str("IndentationError at line "), turbo_call(t_str, 1, (TurboObject*[]){t_line_num}))});
                }
            }
        }
        turbo_call_method(t_self, "tokenize_line", 2, (TurboObject*[]){t_line, t_line_num});
        turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){make_str("NEWLINE"), make_str(""), t_line_num})});
    }
    while (turbo_is_truthy(turbo_gt(turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_self, "indent_stack")}), make_int("1")))) {
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
    TurboObject* t_three_chars = turbo_none;
    TurboObject* t_two_chars = turbo_none;
    TurboObject* t_quote = turbo_none;
    TurboObject* t_val = turbo_none;
    TurboObject* t_ec = turbo_none;
    TurboObject* t_is_float = turbo_none;
    TurboObject* t_has_j = turbo_none;
    TurboObject* t_ch = turbo_none;
    TurboObject* t_char = turbo_none;
    TurboObject* t_is_keyword = turbo_none;
    TurboObject* t_keywords = turbo_none;
    TurboObject* t_k_idx = turbo_none;
    t_p = make_int("0");
    while (turbo_is_truthy(turbo_lt(t_p, turbo_call(t_len, 1, (TurboObject*[]){t_line})))) {
        t_c = turbo_getitem(t_line, t_p);
        if (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = turbo_eq(t_c, make_str(" ")); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("\t")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("\r")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("\n")); }))) {
            t_p = turbo_add(t_p, make_int("1"));
            continue;
        }
        if (turbo_is_truthy(turbo_eq(t_c, make_str("#")))) {
            break;
        }
        if (turbo_is_truthy(turbo_lt(turbo_add(t_p, make_int("2")), turbo_call(t_len, 1, (TurboObject*[]){t_line})))) {
            t_three_chars = turbo_add(turbo_add(turbo_getitem(t_line, t_p), turbo_getitem(t_line, turbo_add(t_p, make_int("1")))), turbo_getitem(t_line, turbo_add(t_p, make_int("2"))));
            if (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = turbo_eq(t_three_chars, make_str("<<=")); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_three_chars, make_str(">>=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_three_chars, make_str("**=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_three_chars, make_str("//=")); }))) {
                turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){t_three_chars, t_three_chars, t_line_num})});
                t_p = turbo_add(t_p, make_int("3"));
                continue;
            }
        }
        if (turbo_is_truthy(turbo_lt(turbo_add(t_p, make_int("1")), turbo_call(t_len, 1, (TurboObject*[]){t_line})))) {
            t_two_chars = turbo_add(turbo_getitem(t_line, t_p), turbo_getitem(t_line, turbo_add(t_p, make_int("1"))));
            if (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = turbo_eq(t_two_chars, make_str("==")); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str("!=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str("<=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str(">=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str("+=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str("-=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str("*=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str("/=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str("%=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str("**")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str("//")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str("<<")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str(">>")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str("&=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str("|=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str("^=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str(":=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_two_chars, make_str("->")); }))) {
                turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){t_two_chars, t_two_chars, t_line_num})});
                t_p = turbo_add(t_p, make_int("2"));
                continue;
            }
        }
        if (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = turbo_eq(t_c, make_str("+")); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("-")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("*")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("/")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("%")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("=")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("<")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str(">")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("(")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str(")")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("[")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("]")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("{")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("}")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str(":")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str(",")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str(".")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("&")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("|")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("^")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("~")); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("@")); }))) {
            turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){t_c, t_c, t_line_num})});
            t_p = turbo_add(t_p, make_int("1"));
            continue;
        }
        if (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = turbo_eq(t_c, make_str("b")); turbo_is_truthy(_lh) ? turbo_lt(turbo_add(t_p, make_int("1")), turbo_call(t_len, 1, (TurboObject*[]){t_line})) : _lh; }); turbo_is_truthy(_lh) ? ({ TurboObject* _lh = turbo_eq(turbo_getitem(t_line, turbo_add(t_p, make_int("1"))), make_str("\"")); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getitem(t_line, turbo_add(t_p, make_int("1"))), make_str("'")); }) : _lh; }))) {
            t_p = turbo_add(t_p, make_int("1"));
            t_quote = turbo_getitem(t_line, t_p);
            t_val = make_str("");
            t_p = turbo_add(t_p, make_int("1"));
            while (turbo_is_truthy(({ TurboObject* _lh = turbo_lt(t_p, turbo_call(t_len, 1, (TurboObject*[]){t_line})); turbo_is_truthy(_lh) ? turbo_ne(turbo_getitem(t_line, t_p), t_quote) : _lh; }))) {
                if (turbo_is_truthy(turbo_eq(turbo_getitem(t_line, t_p), make_str("\\")))) {
                    t_p = turbo_add(t_p, make_int("1"));
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
                    t_p = turbo_add(t_p, make_int("1"));
                } else {
                    t_val = turbo_add(t_val, turbo_getitem(t_line, t_p));
                    t_p = turbo_add(t_p, make_int("1"));
                }
            }
            t_p = turbo_add(t_p, make_int("1"));
            turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){make_str("BYTES"), t_val, t_line_num})});
            continue;
        }
        if (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = turbo_eq(t_c, make_str("f")); turbo_is_truthy(_lh) ? turbo_lt(turbo_add(t_p, make_int("1")), turbo_call(t_len, 1, (TurboObject*[]){t_line})) : _lh; }); turbo_is_truthy(_lh) ? ({ TurboObject* _lh = turbo_eq(turbo_getitem(t_line, turbo_add(t_p, make_int("1"))), make_str("\"")); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getitem(t_line, turbo_add(t_p, make_int("1"))), make_str("'")); }) : _lh; }))) {
            t_p = turbo_add(t_p, make_int("1"));
            t_quote = turbo_getitem(t_line, t_p);
            t_val = make_str("");
            t_p = turbo_add(t_p, make_int("1"));
            while (turbo_is_truthy(({ TurboObject* _lh = turbo_lt(t_p, turbo_call(t_len, 1, (TurboObject*[]){t_line})); turbo_is_truthy(_lh) ? turbo_ne(turbo_getitem(t_line, t_p), t_quote) : _lh; }))) {
                if (turbo_is_truthy(turbo_eq(turbo_getitem(t_line, t_p), make_str("\\")))) {
                    t_p = turbo_add(t_p, make_int("1"));
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
                    t_p = turbo_add(t_p, make_int("1"));
                } else {
                    t_val = turbo_add(t_val, turbo_getitem(t_line, t_p));
                    t_p = turbo_add(t_p, make_int("1"));
                }
            }
            t_p = turbo_add(t_p, make_int("1"));
            turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){make_str("STRING"), t_val, t_line_num})});
            continue;
        }
        if (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = turbo_eq(t_c, make_str("\"")); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("'")); }); turbo_is_truthy(_lh) ? turbo_lt(turbo_add(t_p, make_int("2")), turbo_call(t_len, 1, (TurboObject*[]){t_line})) : _lh; }); turbo_is_truthy(_lh) ? turbo_eq(turbo_getitem(t_line, t_p), turbo_getitem(t_line, turbo_add(t_p, make_int("1")))) : _lh; }); turbo_is_truthy(_lh) ? turbo_eq(turbo_getitem(t_line, t_p), turbo_getitem(t_line, turbo_add(t_p, make_int("2")))) : _lh; }))) {
            t_quote = t_c;
            t_p = turbo_add(t_p, make_int("3"));
            t_val = make_str("");
            while (turbo_is_truthy(turbo_lt(t_p, turbo_call(t_len, 1, (TurboObject*[]){t_line})))) {
                if (turbo_is_truthy(turbo_eq(turbo_getitem(t_line, t_p), make_str("\\")))) {
                    t_p = turbo_add(t_p, make_int("1"));
                    if (turbo_is_truthy(turbo_lt(t_p, turbo_call(t_len, 1, (TurboObject*[]){t_line})))) {
                        t_ec = turbo_getitem(t_line, t_p);
                        if (turbo_is_truthy(turbo_eq(t_ec, make_str("n")))) {
                            turbo_add(t_val, make_str("\n"));
                            turbo_none;
                            turbo_none;
                            turbo_none;
                            turbo_none;
                            make_str("\t");
                            turbo_none;
                            turbo_none;
                            turbo_none;
                            turbo_none;
                            make_str("\r");
                            turbo_none;
                            t_val = turbo_add(t_val, t_ec);
                        }
                        t_p = turbo_add(t_p, make_int("1"));
                    } else {
                        if (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = turbo_eq(turbo_getitem(t_line, t_p), t_quote); turbo_is_truthy(_lh) ? turbo_lt(turbo_add(t_p, make_int("2")), turbo_call(t_len, 1, (TurboObject*[]){t_line})) : _lh; }); turbo_is_truthy(_lh) ? turbo_eq(turbo_getitem(t_line, t_p), turbo_getitem(t_line, turbo_add(t_p, make_int("1")))) : _lh; }); turbo_is_truthy(_lh) ? turbo_eq(turbo_getitem(t_line, t_p), turbo_getitem(t_line, turbo_add(t_p, make_int("2")))) : _lh; }))) {
                            t_p = turbo_add(t_p, make_int("3"));
                            break;
                        } else {
                            t_val = turbo_add(t_val, turbo_getitem(t_line, t_p));
                            t_p = turbo_add(t_p, make_int("1"));
                        }
                    }
                }
                turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){make_str("STRING"), t_val, t_line_num})});
                continue;
            }
            if (turbo_is_truthy(({ TurboObject* _lh = turbo_eq(t_c, make_str("\"")); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("'")); }))) {
                t_quote = t_c;
                t_val = make_str("");
                t_p = turbo_add(t_p, make_int("1"));
                while (turbo_is_truthy(({ TurboObject* _lh = turbo_lt(t_p, turbo_call(t_len, 1, (TurboObject*[]){t_line})); turbo_is_truthy(_lh) ? turbo_ne(turbo_getitem(t_line, t_p), t_quote) : _lh; }))) {
                    if (turbo_is_truthy(turbo_eq(turbo_getitem(t_line, t_p), make_str("\\")))) {
                        t_p = turbo_add(t_p, make_int("1"));
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
                        t_p = turbo_add(t_p, make_int("1"));
                    } else {
                        t_val = turbo_add(t_val, turbo_getitem(t_line, t_p));
                        t_p = turbo_add(t_p, make_int("1"));
                    }
                }
                t_p = turbo_add(t_p, make_int("1"));
                turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){make_str("STRING"), t_val, t_line_num})});
                continue;
            }
            if (turbo_is_truthy(({ TurboObject* _lh = turbo_gte(t_c, make_str("0")); turbo_is_truthy(_lh) ? turbo_lte(t_c, make_str("9")) : _lh; }))) {
                t_val = make_str("");
                t_is_float = turbo_false;
                t_has_j = turbo_false;
                while (turbo_is_truthy(turbo_lt(t_p, turbo_call(t_len, 1, (TurboObject*[]){t_line})))) {
                    t_ch = turbo_getitem(t_line, t_p);
                    if (turbo_is_truthy(({ TurboObject* _lh = turbo_gte(t_ch, make_str("0")); turbo_is_truthy(_lh) ? turbo_lte(t_ch, make_str("9")) : _lh; }))) {
                        t_val = turbo_add(t_val, t_ch);
                        t_p = turbo_add(t_p, make_int("1"));
                    } else {
                        if (turbo_is_truthy(turbo_eq(t_ch, make_str(".")))) {
                            t_is_float = turbo_true;
                            t_val = turbo_add(t_val, t_ch);
                            t_p = turbo_add(t_p, make_int("1"));
                        } else {
                            if (turbo_is_truthy(({ TurboObject* _lh = turbo_eq(t_ch, make_str("e")); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_ch, make_str("E")); }))) {
                                t_is_float = turbo_true;
                                t_val = turbo_add(t_val, t_ch);
                                t_p = turbo_add(t_p, make_int("1"));
                                if (turbo_is_truthy(({ TurboObject* _lh = turbo_lt(t_p, turbo_call(t_len, 1, (TurboObject*[]){t_line})); turbo_is_truthy(_lh) ? ({ TurboObject* _lh = turbo_eq(turbo_getitem(t_line, t_p), make_str("+")); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getitem(t_line, t_p), make_str("-")); }) : _lh; }))) {
                                    t_val = turbo_add(t_val, turbo_getitem(t_line, t_p));
                                    t_p = turbo_add(t_p, make_int("1"));
                                }
                            } else {
                                if (turbo_is_truthy(({ TurboObject* _lh = turbo_eq(t_ch, make_str("j")); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_ch, make_str("J")); }))) {
                                    t_val = turbo_add(t_val, t_ch);
                                    t_p = turbo_add(t_p, make_int("1"));
                                    t_has_j = turbo_true;
                                } else {
                                    break;
                                }
                            }
                        }
                    }
                }
                if (turbo_is_truthy(t_has_j)) {
                    turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){make_str("IMAG"), t_val, t_line_num})});
                } else {
                    if (turbo_is_truthy(t_is_float)) {
                        turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){make_str("FLOAT"), t_val, t_line_num})});
                    } else {
                        turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){make_str("NUMBER"), t_val, t_line_num})});
                    }
                }
                continue;
            }
            if (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = turbo_gte(t_c, make_str("a")); turbo_is_truthy(_lh) ? turbo_lte(t_c, make_str("z")) : _lh; }); turbo_is_truthy(_lh) ? _lh : ({ TurboObject* _lh = turbo_gte(t_c, make_str("A")); turbo_is_truthy(_lh) ? turbo_lte(t_c, make_str("Z")) : _lh; }); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_c, make_str("_")); }))) {
                t_val = make_str("");
                while (turbo_is_truthy(turbo_lt(t_p, turbo_call(t_len, 1, (TurboObject*[]){t_line})))) {
                    t_char = turbo_getitem(t_line, t_p);
                    if (turbo_is_truthy(({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = ({ TurboObject* _lh = turbo_gte(t_char, make_str("a")); turbo_is_truthy(_lh) ? turbo_lte(t_char, make_str("z")) : _lh; }); turbo_is_truthy(_lh) ? _lh : ({ TurboObject* _lh = turbo_gte(t_char, make_str("A")); turbo_is_truthy(_lh) ? turbo_lte(t_char, make_str("Z")) : _lh; }); }); turbo_is_truthy(_lh) ? _lh : ({ TurboObject* _lh = turbo_gte(t_char, make_str("0")); turbo_is_truthy(_lh) ? turbo_lte(t_char, make_str("9")) : _lh; }); }); turbo_is_truthy(_lh) ? _lh : turbo_eq(t_char, make_str("_")); }))) {
                        t_val = turbo_add(t_val, t_char);
                        t_p = turbo_add(t_p, make_int("1"));
                    } else {
                        break;
                    }
                }
                t_is_keyword = turbo_false;
                t_keywords = ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, make_str("def")); turbo_list_append(_lst, make_str("class")); turbo_list_append(_lst, make_str("if")); turbo_list_append(_lst, make_str("elif")); turbo_list_append(_lst, make_str("else")); turbo_list_append(_lst, make_str("while")); turbo_list_append(_lst, make_str("for")); turbo_list_append(_lst, make_str("in")); turbo_list_append(_lst, make_str("return")); turbo_list_append(_lst, make_str("and")); turbo_list_append(_lst, make_str("or")); turbo_list_append(_lst, make_str("not")); turbo_list_append(_lst, make_str("pass")); turbo_list_append(_lst, make_str("None")); turbo_list_append(_lst, make_str("True")); turbo_list_append(_lst, make_str("False")); turbo_list_append(_lst, make_str("import")); turbo_list_append(_lst, make_str("break")); turbo_list_append(_lst, make_str("continue")); turbo_list_append(_lst, make_str("raise")); turbo_list_append(_lst, make_str("assert")); turbo_list_append(_lst, make_str("del")); turbo_list_append(_lst, make_str("global")); turbo_list_append(_lst, make_str("nonlocal")); turbo_list_append(_lst, make_str("try")); turbo_list_append(_lst, make_str("except")); turbo_list_append(_lst, make_str("finally")); turbo_list_append(_lst, make_str("with")); turbo_list_append(_lst, make_str("as")); turbo_list_append(_lst, make_str("from")); turbo_list_append(_lst, make_str("async")); turbo_list_append(_lst, make_str("await")); turbo_list_append(_lst, make_str("match")); turbo_list_append(_lst, make_str("case")); turbo_list_append(_lst, make_str("yield")); turbo_list_append(_lst, make_str("lambda"));  _lst; });
                t_k_idx = make_int("0");
                while (turbo_is_truthy(turbo_lt(t_k_idx, turbo_call(t_len, 1, (TurboObject*[]){t_keywords})))) {
                    if (turbo_is_truthy(turbo_eq(turbo_getitem(t_keywords, t_k_idx), t_val))) {
                        t_is_keyword = turbo_true;
                        break;
                    }
                    t_k_idx = turbo_add(t_k_idx, make_int("1"));
                }
                if (turbo_is_truthy(t_is_keyword)) {
                    turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){t_val, t_val, t_line_num})});
                } else {
                    turbo_call_method(turbo_getattr(t_self, "tokens"), "append", 1, (TurboObject*[]){turbo_call(t_Token, 3, (TurboObject*[]){make_str("NAME"), t_val, t_line_num})});
                }
                continue;
            }
            turbo_call(t_print, 1, (TurboObject*[]){turbo_add(turbo_add(turbo_add(make_str("Lexer error: unexpected character "), t_c), make_str(" at line ")), turbo_call(t_str, 1, (TurboObject*[]){t_line_num}))});
            t_p = turbo_add(t_p, make_int("1"));
        }
    }
    return turbo_none;
}

TurboObject* t_impl_Lexer_collect_locals(int argc, TurboObject** args) {
    TurboObject* t_node = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_locals_list = (argc > 1) ? args[1] : turbo_none;
    TurboObject* t_target = turbo_none;
    TurboObject* t_name = turbo_none;
    TurboObject* t_found = turbo_none;
    TurboObject* t_i = turbo_none;
    TurboObject* t_exc_var = turbo_none;
    TurboObject* t_alias_node = turbo_none;
    TurboObject* t_idx = turbo_none;
    if (turbo_is_truthy(({ TurboObject* _lh = turbo_eq(turbo_getattr(t_node, "type"), make_str("ASSIGN")); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getattr(t_node, "type"), make_str("AUGASSIGN")); }))) {
        t_target = turbo_getitem(turbo_getattr(t_node, "children"), make_int("0"));
        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_target, "type"), make_str("NAME")))) {
            t_name = turbo_getattr(t_target, "value");
            t_found = turbo_false;
            t_i = make_int("0");
            while (turbo_is_truthy(turbo_lt(t_i, turbo_call(t_len, 1, (TurboObject*[]){t_locals_list})))) {
                if (turbo_is_truthy(turbo_eq(turbo_getitem(t_locals_list, t_i), t_name))) {
                    t_found = turbo_true;
                    break;
                }
                t_i = turbo_add(t_i, make_int("1"));
            }
            if (turbo_is_truthy(turbo_not(t_found))) {
                turbo_call_method(t_locals_list, "append", 1, (TurboObject*[]){t_name});
            }
        }
    } else {
        if (turbo_is_truthy(({ TurboObject* _lh = turbo_eq(turbo_getattr(t_node, "type"), make_str("FOR")); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getattr(t_node, "type"), make_str("ASYNC_FOR")); }))) {
            t_name = turbo_getattr(t_node, "value");
            t_found = turbo_false;
            t_i = make_int("0");
            while (turbo_is_truthy(turbo_lt(t_i, turbo_call(t_len, 1, (TurboObject*[]){t_locals_list})))) {
                if (turbo_is_truthy(turbo_eq(turbo_getitem(t_locals_list, t_i), t_name))) {
                    t_found = turbo_true;
                    break;
                }
                t_i = turbo_add(t_i, make_int("1"));
            }
            if (turbo_is_truthy(turbo_not(t_found))) {
                turbo_call_method(t_locals_list, "append", 1, (TurboObject*[]){t_name});
            }
        }
    }
    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("EXCEPT")))) {
        t_exc_var = turbo_getattr(t_node, "value");
        if (turbo_is_truthy(turbo_ne(t_exc_var, make_str("")))) {
            t_found = turbo_false;
            t_i = make_int("0");
            while (turbo_is_truthy(turbo_lt(t_i, turbo_call(t_len, 1, (TurboObject*[]){t_locals_list})))) {
                if (turbo_is_truthy(turbo_eq(turbo_getitem(t_locals_list, t_i), t_exc_var))) {
                    t_found = turbo_true;
                    break;
                }
                t_i = turbo_add(t_i, make_int("1"));
            }
            if (turbo_is_truthy(turbo_not(t_found))) {
                turbo_call_method(t_locals_list, "append", 1, (TurboObject*[]){t_exc_var});
            }
        }
    }
    if (turbo_is_truthy(({ TurboObject* _lh = turbo_eq(turbo_getattr(t_node, "type"), make_str("WITH")); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getattr(t_node, "type"), make_str("ASYNC_WITH")); }))) {
        t_alias_node = turbo_getitem(turbo_getattr(t_node, "children"), make_int("1"));
        if (turbo_is_truthy(turbo_eq(turbo_getattr(t_alias_node, "type"), make_str("NAME")))) {
            t_name = turbo_getattr(t_alias_node, "value");
            t_found = turbo_false;
            t_i = make_int("0");
            while (turbo_is_truthy(turbo_lt(t_i, turbo_call(t_len, 1, (TurboObject*[]){t_locals_list})))) {
                if (turbo_is_truthy(turbo_eq(turbo_getitem(t_locals_list, t_i), t_name))) {
                    t_found = turbo_true;
                    break;
                }
                t_i = turbo_add(t_i, make_int("1"));
            }
            if (turbo_is_truthy(turbo_not(t_found))) {
                turbo_call_method(t_locals_list, "append", 1, (TurboObject*[]){t_name});
            }
        }
    }
    if (turbo_is_truthy(turbo_eq(turbo_getattr(t_node, "type"), make_str("PATTERN_CAP")))) {
        t_name = turbo_getattr(t_node, "value");
        t_found = turbo_false;
        t_i = make_int("0");
        while (turbo_is_truthy(turbo_lt(t_i, turbo_call(t_len, 1, (TurboObject*[]){t_locals_list})))) {
            if (turbo_is_truthy(turbo_eq(turbo_getitem(t_locals_list, t_i), t_name))) {
                t_found = turbo_true;
                break;
            }
            t_i = turbo_add(t_i, make_int("1"));
        }
        if (turbo_is_truthy(turbo_not(t_found))) {
            turbo_call_method(t_locals_list, "append", 1, (TurboObject*[]){t_name});
        }
    }
    if (turbo_is_truthy(({ TurboObject* _lh = turbo_eq(turbo_getattr(t_node, "type"), make_str("DEF")); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getattr(t_node, "type"), make_str("ASYNC_DEF")); }))) {
        return turbo_none;
    }
    t_idx = make_int("0");
    while (turbo_is_truthy(turbo_lt(t_idx, turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_node, "children")})))) {
        turbo_call(t_collect_locals, 2, (TurboObject*[]){turbo_getitem(turbo_getattr(t_node, "children"), t_idx), t_locals_list});
        t_idx = turbo_add(t_idx, make_int("1"));
    }
    return turbo_none;
}

TurboObject* t_impl_Lexer_collect_globals(int argc, TurboObject** args) {
    TurboObject* t_block_node = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_globals_list = (argc > 1) ? args[1] : turbo_none;
    TurboObject* t_stmts = turbo_none;
    TurboObject* t_idx = turbo_none;
    TurboObject* t_node = turbo_none;
    t_stmts = turbo_getattr(t_block_node, "children");
    t_idx = make_int("0");
    while (turbo_is_truthy(turbo_lt(t_idx, turbo_call(t_len, 1, (TurboObject*[]){t_stmts})))) {
        t_node = turbo_getitem(t_stmts, t_idx);
        turbo_call(t_collect_locals, 2, (TurboObject*[]){t_node, t_globals_list});
        t_idx = turbo_add(t_idx, make_int("1"));
    }
    return turbo_none;
}

TurboObject* t_impl_Lexer_scan_global_nonlocal_names(int argc, TurboObject** args) {
    TurboObject* t_body_node = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_names = turbo_none;
    TurboObject* t_stmts = turbo_none;
    TurboObject* t_idx = turbo_none;
    TurboObject* t_stmt = turbo_none;
    TurboObject* t_list_node = turbo_none;
    TurboObject* t_n_idx = turbo_none;
    TurboObject* t_name_node = turbo_none;
    t_names = make_list();
    t_stmts = turbo_getattr(t_body_node, "children");
    t_idx = make_int("0");
    while (turbo_is_truthy(turbo_lt(t_idx, turbo_call(t_len, 1, (TurboObject*[]){t_stmts})))) {
        t_stmt = turbo_getitem(t_stmts, t_idx);
        if (turbo_is_truthy(({ TurboObject* _lh = turbo_eq(turbo_getattr(t_stmt, "type"), make_str("GLOBAL")); turbo_is_truthy(_lh) ? _lh : turbo_eq(turbo_getattr(t_stmt, "type"), make_str("NONLOCAL")); }))) {
            t_list_node = turbo_getitem(turbo_getattr(t_stmt, "children"), make_int("0"));
            t_n_idx = make_int("0");
            while (turbo_is_truthy(turbo_lt(t_n_idx, turbo_call(t_len, 1, (TurboObject*[]){turbo_getattr(t_list_node, "children")})))) {
                t_name_node = turbo_getitem(turbo_getattr(t_list_node, "children"), t_n_idx);
                if (turbo_is_truthy(turbo_eq(turbo_getattr(t_name_node, "type"), make_str("NAME")))) {
                    turbo_call_method(t_names, "append", 1, (TurboObject*[]){turbo_getattr(t_name_node, "value")});
                }
                t_n_idx = turbo_add(t_n_idx, make_int("1"));
            }
        }
        t_idx = turbo_add(t_idx, make_int("1"));
    }
    return t_names;
    return turbo_none;
}

TurboObject* t_impl_Lexer_escape_c_string(int argc, TurboObject** args) {
    TurboObject* t_s = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_res = turbo_none;
    TurboObject* t_i = turbo_none;
    TurboObject* t_c = turbo_none;
    t_res = make_str("");
    t_i = make_int("0");
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
        t_i = turbo_add(t_i, make_int("1"));
    }
    return t_res;
    return turbo_none;
}

TurboObject* t_impl_Lexer_main(int argc, TurboObject** args) {
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
    if (turbo_is_truthy(turbo_lt(turbo_call(t_len, 1, (TurboObject*[]){t_sys_argv}), make_int("4")))) {
        turbo_call(t_print, 1, (TurboObject*[]){make_str("Usage: turbo <input_file> -o <output_file>")});
        return turbo_none;
    }
    t_input_file = turbo_getitem(t_sys_argv, make_int("1"));
    t_output_file = make_str("");
    t_i = make_int("2");
    while (turbo_is_truthy(turbo_lt(t_i, turbo_call(t_len, 1, (TurboObject*[]){t_sys_argv})))) {
        if (turbo_is_truthy(turbo_eq(turbo_getitem(t_sys_argv, t_i), make_str("-o")))) {
            if (turbo_is_truthy(turbo_lt(turbo_add(t_i, make_int("1")), turbo_call(t_len, 1, (TurboObject*[]){t_sys_argv})))) {
                t_output_file = turbo_getitem(t_sys_argv, turbo_add(t_i, make_int("1")));
                break;
            }
        }
        t_i = turbo_add(t_i, make_int("1"));
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
    t_g_idx = make_int("0");
    t_builtins = ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, make_str("print")); turbo_list_append(_lst, make_str("len")); turbo_list_append(_lst, make_str("str")); turbo_list_append(_lst, make_str("int")); turbo_list_append(_lst, make_str("ord")); turbo_list_append(_lst, make_str("chr")); turbo_list_append(_lst, make_str("range")); turbo_list_append(_lst, make_str("open")); turbo_list_append(_lst, make_str("sys_argv")); turbo_list_append(_lst, make_str("input")); turbo_list_append(_lst, make_str("type")); turbo_list_append(_lst, make_str("isinstance")); turbo_list_append(_lst, make_str("hasattr")); turbo_list_append(_lst, make_str("getattr")); turbo_list_append(_lst, make_str("setattr")); turbo_list_append(_lst, make_str("repr")); turbo_list_append(_lst, make_str("abs")); turbo_list_append(_lst, make_str("round")); turbo_list_append(_lst, make_str("pow")); turbo_list_append(_lst, make_str("hex")); turbo_list_append(_lst, make_str("bin")); turbo_list_append(_lst, make_str("oct")); turbo_list_append(_lst, make_str("float")); turbo_list_append(_lst, make_str("bool")); turbo_list_append(_lst, make_str("list")); turbo_list_append(_lst, make_str("dict")); turbo_list_append(_lst, make_str("super")); turbo_list_append(_lst, make_str("iter")); turbo_list_append(_lst, make_str("next")); turbo_list_append(_lst, make_str("all")); turbo_list_append(_lst, make_str("any")); turbo_list_append(_lst, make_str("sum")); turbo_list_append(_lst, make_str("min")); turbo_list_append(_lst, make_str("max")); turbo_list_append(_lst, make_str("sorted")); turbo_list_append(_lst, make_str("reversed")); turbo_list_append(_lst, make_str("enumerate")); turbo_list_append(_lst, make_str("zip")); turbo_list_append(_lst, make_str("map")); turbo_list_append(_lst, make_str("filter"));  _lst; });
    while (turbo_is_truthy(turbo_lt(t_g_idx, turbo_call(t_len, 1, (TurboObject*[]){t_globals_list})))) {
        t_g_name = turbo_getitem(t_globals_list, t_g_idx);
        t_is_builtin = turbo_false;
        t_b_idx = make_int("0");
        while (turbo_is_truthy(turbo_lt(t_b_idx, turbo_call(t_len, 1, (TurboObject*[]){t_builtins})))) {
            if (turbo_is_truthy(turbo_eq(turbo_getitem(t_builtins, t_b_idx), t_g_name))) {
                t_is_builtin = turbo_true;
                break;
            }
            t_b_idx = turbo_add(t_b_idx, make_int("1"));
        }
        if (turbo_is_truthy(turbo_not(t_is_builtin))) {
            turbo_call_method(t_cg, "write_header", 1, (TurboObject*[]){turbo_add(turbo_add(make_str("TurboObject* t_"), t_g_name), make_str(" = NULL;\n"))});
        }
        t_g_idx = turbo_add(t_g_idx, make_int("1"));
    }
    turbo_setattr(t_cg, "indent_level", make_int("1"));
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
    turbo_class_add_method(t_Lexer, "collect_locals", t_impl_Lexer_collect_locals);
    turbo_class_add_method(t_Lexer, "collect_globals", t_impl_Lexer_collect_globals);
    turbo_class_add_method(t_Lexer, "scan_global_nonlocal_names", t_impl_Lexer_scan_global_nonlocal_names);
    turbo_class_add_method(t_Lexer, "escape_c_string", t_impl_Lexer_escape_c_string);
    turbo_class_add_method(t_Lexer, "main", t_impl_Lexer_main);
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
