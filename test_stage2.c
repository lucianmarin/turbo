#include "runtime.h"
#include <stdlib.h>

TurboObject* t_impl_test_arithmetic(int argc, TurboObject** args);
TurboObject* t_test_arithmetic = NULL;
TurboObject* t_impl_test_comparisons(int argc, TurboObject** args);
TurboObject* t_test_comparisons = NULL;
TurboObject* t_impl_test_lists(int argc, TurboObject** args);
TurboObject* t_test_lists = NULL;
TurboObject* t_impl_test_strings(int argc, TurboObject** args);
TurboObject* t_test_strings = NULL;
TurboObject* t_impl_test_loops(int argc, TurboObject** args);
TurboObject* t_test_loops = NULL;
TurboObject* t_Greeter = NULL;
TurboObject* t_impl_Greeter___init__(int argc, TurboObject** args);
TurboObject* t_impl_Greeter_greet(int argc, TurboObject** args);
TurboObject* t_impl_test_classes(int argc, TurboObject** args);
TurboObject* t_test_classes = NULL;
TurboObject* t_impl_main(int argc, TurboObject** args);
TurboObject* t_main = NULL;

TurboObject* t_impl_test_arithmetic(int argc, TurboObject** args) {
    TurboObject* t_x = turbo_none;
    TurboObject* t_y = turbo_none;
    t_x = make_int(10);
    t_y = make_int(3);
    turbo_call(t_print, 1, (TurboObject*[]){make_str("Arithmetic:")});
    turbo_call(t_print, 2, (TurboObject*[]){make_str("x + y ="), turbo_add(t_x, t_y)});
    turbo_call(t_print, 2, (TurboObject*[]){make_str("x - y ="), turbo_sub(t_x, t_y)});
    turbo_call(t_print, 2, (TurboObject*[]){make_str("x * y ="), turbo_mul(t_x, t_y)});
    turbo_call(t_print, 2, (TurboObject*[]){make_str("x / y ="), turbo_div(t_x, t_y)});
    turbo_call(t_print, 2, (TurboObject*[]){make_str("x % y ="), turbo_mod(t_x, t_y)});
    return turbo_none;
}

TurboObject* t_impl_test_comparisons(int argc, TurboObject** args) {
    TurboObject* t_x = turbo_none;
    TurboObject* t_y = turbo_none;
    turbo_call(t_print, 1, (TurboObject*[]){make_str("Comparisons:")});
    t_x = make_int(5);
    t_y = make_int(10);
    turbo_call(t_print, 2, (TurboObject*[]){make_str("x < y:"), turbo_lt(t_x, t_y)});
    turbo_call(t_print, 2, (TurboObject*[]){make_str("x > y:"), turbo_gt(t_x, t_y)});
    turbo_call(t_print, 2, (TurboObject*[]){make_str("x == y:"), turbo_eq(t_x, t_y)});
    turbo_call(t_print, 2, (TurboObject*[]){make_str("x != y:"), turbo_ne(t_x, t_y)});
    return turbo_none;
}

TurboObject* t_impl_test_lists(int argc, TurboObject** args) {
    TurboObject* t_lst = turbo_none;
    turbo_call(t_print, 1, (TurboObject*[]){make_str("Lists:")});
    t_lst = ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, make_int(1)); turbo_list_append(_lst, make_int(2));  _lst; });
    turbo_call_method(t_lst, "append", 1, (TurboObject*[]){make_int(3)});
    turbo_call_method(t_lst, "append", 1, (TurboObject*[]){make_int(4)});
    turbo_call(t_print, 2, (TurboObject*[]){make_str("lst:"), t_lst});
    turbo_call(t_print, 2, (TurboObject*[]){make_str("len(lst):"), turbo_call(t_len, 1, (TurboObject*[]){t_lst})});
    turbo_call(t_print, 2, (TurboObject*[]){make_str("lst[0]:"), turbo_getitem(t_lst, make_int(0))});
    turbo_call(t_print, 2, (TurboObject*[]){make_str("lst[-1]:"), turbo_getitem(t_lst, turbo_sub(make_int(0), make_int(1)))});
    turbo_call(t_print, 2, (TurboObject*[]){make_str("lst[1:3]:"), turbo_slice(t_lst, make_int(1), make_int(3))});
    return turbo_none;
}

TurboObject* t_impl_test_strings(int argc, TurboObject** args) {
    TurboObject* t_s = turbo_none;
    TurboObject* t_parts = turbo_none;
    turbo_call(t_print, 1, (TurboObject*[]){make_str("Strings:")});
    t_s = turbo_add(make_str("hello"), make_str(" world"));
    turbo_call(t_print, 2, (TurboObject*[]){make_str("s:"), t_s});
    turbo_call(t_print, 2, (TurboObject*[]){make_str("len(s):"), turbo_call(t_len, 1, (TurboObject*[]){t_s})});
    turbo_call(t_print, 2, (TurboObject*[]){make_str("s[0]:"), turbo_getitem(t_s, make_int(0))});
    turbo_call(t_print, 2, (TurboObject*[]){make_str("s[6:11]:"), turbo_slice(t_s, make_int(6), make_int(11))});
    t_parts = turbo_call_method(t_s, "split", 0, NULL);
    turbo_call(t_print, 2, (TurboObject*[]){make_str("split parts:"), t_parts});
    return turbo_none;
}

TurboObject* t_impl_test_loops(int argc, TurboObject** args) {
    TurboObject* t_i = turbo_none;
    TurboObject* t_j = turbo_none;
    TurboObject* t_item = turbo_none;
    turbo_call(t_print, 1, (TurboObject*[]){make_str("Loops:")});
    t_i = make_int(0);
    while (turbo_is_truthy(turbo_lt(t_i, make_int(3)))) {
        turbo_call(t_print, 2, (TurboObject*[]){make_str("while loop count:"), t_i});
        t_i = turbo_add(t_i, make_int(1));
    }
    {
        TurboObject* _iter = turbo_call(t_range, 1, (TurboObject*[]){make_int(3)});
        if (_iter->type == TYPE_LIST) {
            for (int _i = 0; _i < _iter->list_val.length; _i++) {
                t_j = _iter->list_val.items[_i];
                turbo_call(t_print, 2, (TurboObject*[]){make_str("for j in range(3):"), t_j});
            }
        } else if (_iter->type == TYPE_STR) {
            for (int _i = 0; _i < _iter->str_val.length; _i++) {
                char _tmp[2] = {_iter->str_val.chars[_i], '\0'};
                t_j = make_str(_tmp);
                turbo_call(t_print, 2, (TurboObject*[]){make_str("for j in range(3):"), t_j});
            }
        }
    }
    {
        TurboObject* _iter = ({ TurboObject* _lst = make_list(); turbo_list_append(_lst, make_str("apple")); turbo_list_append(_lst, make_str("banana"));  _lst; });
        if (_iter->type == TYPE_LIST) {
            for (int _i = 0; _i < _iter->list_val.length; _i++) {
                t_item = _iter->list_val.items[_i];
                turbo_call(t_print, 2, (TurboObject*[]){make_str("for item in list:"), t_item});
            }
        } else if (_iter->type == TYPE_STR) {
            for (int _i = 0; _i < _iter->str_val.length; _i++) {
                char _tmp[2] = {_iter->str_val.chars[_i], '\0'};
                t_item = make_str(_tmp);
                turbo_call(t_print, 2, (TurboObject*[]){make_str("for item in list:"), t_item});
            }
        }
    }
    return turbo_none;
}

TurboObject* t_impl_Greeter___init__(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_prefix = (argc > 1) ? args[1] : turbo_none;
    turbo_setattr(t_self, "prefix", t_prefix);
    return turbo_none;
}

TurboObject* t_impl_Greeter_greet(int argc, TurboObject** args) {
    TurboObject* t_self = (argc > 0) ? args[0] : turbo_none;
    TurboObject* t_target = (argc > 1) ? args[1] : turbo_none;
    turbo_call(t_print, 1, (TurboObject*[]){turbo_add(turbo_add(turbo_add(turbo_getattr(t_self, "prefix"), make_str(", ")), t_target), make_str("!"))});
    return turbo_none;
}

TurboObject* t_impl_test_classes(int argc, TurboObject** args) {
    TurboObject* t_g = turbo_none;
    turbo_call(t_print, 1, (TurboObject*[]){make_str("Classes:")});
    t_g = turbo_call(t_Greeter, 1, (TurboObject*[]){make_str("Hello")});
    turbo_call_method(t_g, "greet", 1, (TurboObject*[]){make_str("Turbo")});
    turbo_call(t_print, 2, (TurboObject*[]){make_str("g.prefix attribute access:"), turbo_getattr(t_g, "prefix")});
    return turbo_none;
}

TurboObject* t_impl_main(int argc, TurboObject** args) {
    turbo_call(t_test_arithmetic, 0, NULL);
    turbo_call(t_test_comparisons, 0, NULL);
    turbo_call(t_test_lists, 0, NULL);
    turbo_call(t_test_strings, 0, NULL);
    turbo_call(t_test_loops, 0, NULL);
    turbo_call(t_test_classes, 0, NULL);
    return turbo_none;
}


void turbo_main(void) {
    t_test_arithmetic = make_func(t_impl_test_arithmetic, "test_arithmetic");
    t_test_comparisons = make_func(t_impl_test_comparisons, "test_comparisons");
    t_test_lists = make_func(t_impl_test_lists, "test_lists");
    t_test_strings = make_func(t_impl_test_strings, "test_strings");
    t_test_loops = make_func(t_impl_test_loops, "test_loops");
    t_Greeter = make_class("Greeter");
    turbo_class_add_method(t_Greeter, "__init__", t_impl_Greeter___init__);
    turbo_class_add_method(t_Greeter, "greet", t_impl_Greeter_greet);
    t_test_classes = make_func(t_impl_test_classes, "test_classes");
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
