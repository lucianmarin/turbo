# Turbo Language Compiler
# Matches Python syntax 1 to 1 but compiles to C.

import sys
sys_argv = sys.argv

class Token:
    def __init__(self, type, value, line):
        self.type = type
        self.value = value
        self.line = line

class ASTNode:
    def __init__(self, type, value, children):
        self.type = type
        self.value = value
        self.children = children

def count_leading_spaces(line):
    count = 0
    while count < len(line):
        c = line[count]
        if c == ' ':
            count = count + 1
        elif c == '\t':
            count = count + 4
        else:
            break
    return count

def is_empty_or_comment(line):
    i = 0
    while i < len(line):
        c = line[i]
        if c == ' ' or c == '\t' or c == '\r' or c == '\n':
            i = i + 1
        elif c == '#':
            return True
        else:
            return False
    return True

class Lexer:
    def __init__(self, text):
        self.text = text
        self.indent_stack = [0]
        self.tokens = []

    def tokenize(self):
        lines = self.text.split("\n")
        line_idx = 0
        while line_idx < len(lines):
            line = lines[line_idx]
            line_num = line_idx + 1
            line_idx = line_idx + 1
            
            # Remove trailing carriage return
            if len(line) > 0:
                if line[len(line)-1] == '\r':
                    line = line[0:len(line)-1]
            
            if is_empty_or_comment(line):
                continue
            
            spaces = count_leading_spaces(line)
            current_indent = self.indent_stack[len(self.indent_stack)-1]
            
            if spaces > current_indent:
                self.indent_stack.append(spaces)
                self.tokens.append(Token("INDENT", "", line_num))
            elif spaces < current_indent:
                while len(self.indent_stack) > 0 and self.indent_stack[len(self.indent_stack)-1] > spaces:
                    self.indent_stack.pop()
                    self.tokens.append(Token("DEDENT", "", line_num))
                if self.indent_stack[len(self.indent_stack)-1] != spaces:
                    print("IndentationError at line " + str(line_num))
            
            self.tokenize_line(line, line_num)
            self.tokens.append(Token("NEWLINE", "", line_num))
            
        while len(self.indent_stack) > 1:
            self.indent_stack.pop()
            self.tokens.append(Token("DEDENT", "", line_idx))
        self.tokens.append(Token("EOF", "", line_idx))

    def tokenize_line(self, line, line_num):
        p = 0
        while p < len(line):
            c = line[p]
            if c == ' ' or c == '\t' or c == '\r' or c == '\n':
                p = p + 1
                continue
            if c == '#':
                break
            
            # Multi-character operators (check 3-char first, then 2-char)
            if p + 2 < len(line):
                three_chars = line[p] + line[p+1] + line[p+2]
                if three_chars == '<<=' or three_chars == '>>=' or three_chars == '**=' or three_chars == '//=':
                    self.tokens.append(Token(three_chars, three_chars, line_num))
                    p = p + 3
                    continue
            if p + 1 < len(line):
                two_chars = line[p] + line[p+1]
                if two_chars == '==' or two_chars == '!=' or two_chars == '<=' or two_chars == '>=' or two_chars == '+=' or two_chars == '-=' or two_chars == '*=' or two_chars == '/=' or two_chars == '%=' or two_chars == '**' or two_chars == '//' or two_chars == '<<' or two_chars == '>>' or two_chars == '&=' or two_chars == '|=' or two_chars == '^=' or two_chars == ':=' or two_chars == '->' or two_chars == '@=':
                    self.tokens.append(Token(two_chars, two_chars, line_num))
                    p = p + 2
                    continue
            
            # Single-character operators
            if c == '+' or c == '-' or c == '*' or c == '/' or c == '%' or c == '=' or c == '<' or c == '>' or c == '(' or c == ')' or c == '[' or c == ']' or c == '{' or c == '}' or c == ':' or c == ',' or c == '.' or c == '&' or c == '|' or c == '^' or c == '~' or c == '@':
                self.tokens.append(Token(c, c, line_num))
                p = p + 1
                continue
            
            # Bytes literals b"..."
            if c == 'b' and p + 1 < len(line) and (line[p+1] == '"' or line[p+1] == "'"):
                p = p + 1
                quote = line[p]
                val = ""
                p = p + 1
                while p < len(line) and line[p] != quote:
                    if line[p] == '\\':
                        p = p + 1
                        if p < len(line):
                            ec = line[p]
                            if ec == 'n':
                                val = val + "\n"
                            elif ec == 't':
                                val = val + "\t"
                            elif ec == 'r':
                                val = val + "\r"
                            else:
                                val = val + ec
                        p = p + 1
                    else:
                        val = val + line[p]
                        p = p + 1
                p = p + 1
                self.tokens.append(Token("BYTES", val, line_num))
                continue

            # f-strings
            if c == 'f' and p + 1 < len(line) and (line[p+1] == '"' or line[p+1] == "'"):
                p = p + 1
                quote = line[p]
                val = ""
                p = p + 1
                while p < len(line) and line[p] != quote:
                    if line[p] == '\\':
                        p = p + 1
                        if p < len(line):
                            ec = line[p]
                            if ec == 'n':
                                val = val + "\n"
                            elif ec == 't':
                                val = val + "\t"
                            elif ec == 'r':
                                val = val + "\r"
                            else:
                                val = val + ec
                        p = p + 1
                    else:
                        val = val + line[p]
                        p = p + 1
                p = p + 1
                self.tokens.append(Token("STRING", val, line_num))
                continue
            
            # Triple-quoted strings """ or '''
            if (c == '"' or c == "'") and p + 2 < len(line) and line[p] == line[p+1] and line[p] == line[p+2]:
                quote = c
                p = p + 3
                val = ""
                while p < len(line):
                    if line[p] == '\\':
                        p = p + 1
                        if p < len(line):
                            ec = line[p]
                            if ec == 'n':
                                val = val + "\n"
                            elif ec == 't':
                                val = val + "\t"
                            elif ec == 'r':
                                val = val + "\r"
                            else:
                                val = val + ec
                        p = p + 1
                    elif line[p] == quote and p + 2 < len(line) and line[p] == line[p+1] and line[p] == line[p+2]:
                        p = p + 3
                        break
                    else:
                        val = val + line[p]
                        p = p + 1
                self.tokens.append(Token("STRING", val, line_num))
                continue
            
            # String literals
            if c == '"' or c == "'":
                quote = c
                val = ""
                p = p + 1
                while p < len(line) and line[p] != quote:
                    if line[p] == '\\':
                        p = p + 1
                        if p < len(line):
                            ec = line[p]
                            if ec == 'n':
                                val = val + "\n"
                            elif ec == 't':
                                val = val + "\t"
                            elif ec == 'r':
                                val = val + "\r"
                            else:
                                val = val + ec
                        p = p + 1
                    else:
                        val = val + line[p]
                        p = p + 1
                p = p + 1
                self.tokens.append(Token("STRING", val, line_num))
                continue
            
            # Numeric literals (int, float, and complex imaginary)
            if c >= '0' and c <= '9':
                val = ""
                is_float = False
                has_j = False
                while p < len(line):
                    ch = line[p]
                    if ch >= '0' and ch <= '9':
                        val = val + ch
                        p = p + 1
                    elif ch == '.':
                        is_float = True
                        val = val + ch
                        p = p + 1
                    elif ch == 'e' or ch == 'E':
                        is_float = True
                        val = val + ch
                        p = p + 1
                        if p < len(line) and (line[p] == '+' or line[p] == '-'):
                            val = val + line[p]
                            p = p + 1
                    elif ch == 'j' or ch == 'J':
                        val = val + ch
                        p = p + 1
                        has_j = True
                    else:
                        break
                if has_j:
                    self.tokens.append(Token("IMAG", val, line_num))
                elif is_float:
                    self.tokens.append(Token("FLOAT", val, line_num))
                else:
                    self.tokens.append(Token("NUMBER", val, line_num))
                continue
            
            # Identifiers / Keywords
            if (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z') or c == '_':
                val = ""
                while p < len(line):
                    char = line[p]
                    if (char >= 'a' and char <= 'z') or (char >= 'A' and char <= 'Z') or (char >= '0' and char <= '9') or char == '_':
                        val = val + char
                        p = p + 1
                    else:
                        break
                
                is_keyword = False
                keywords = ["def", "class", "if", "elif", "else", "while", "for", "in", "return", "and", "or", "not", "pass", "None", "True", "False", "import", "break", "continue", "raise", "assert", "del", "global", "nonlocal", "try", "except", "finally", "with", "as", "from", "async", "await", "match", "case", "yield", "lambda"]
                k_idx = 0
                while k_idx < len(keywords):
                    if keywords[k_idx] == val:
                        is_keyword = True
                        break
                    k_idx = k_idx + 1
                
                if is_keyword:
                    self.tokens.append(Token(val, val, line_num))
                else:
                    self.tokens.append(Token("NAME", val, line_num))
                continue
            
            print("Lexer error: unexpected character " + c + " at line " + str(line_num))
            p = p + 1

class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0

    def peek(self):
        if self.pos >= len(self.tokens):
            return Token("EOF", "", -1)
        return self.tokens[self.pos]

    def consume(self, expected_type):
        t = self.peek()
        if t.type == expected_type:
            self.pos = self.pos + 1
            return t
        print("Parser error: expected " + expected_type + " but got " + t.type + " at line " + str(t.line))
        self.pos = self.pos + 1
        return t

    def try_match(self, token_type):
        t = self.peek()
        if t.type == token_type:
            self.pos = self.pos + 1
            return True
        return False

    def parse_expr(self):
        node = self.parse_and()
        while self.try_match("or"):
            right = self.parse_and()
            node = ASTNode("OR", "", [node, right])
        if self.try_match("if"):
            cond = self.parse_and()
            self.consume("else")
            else_val = self.parse_expr()
            node = ASTNode("TERNARY", "", [cond, node, else_val])
        if self.try_match(':='):
            value = self.parse_expr()
            node = ASTNode("ASSIGN", "", [node, value])
        return node

    def parse_and(self):
        node = self.parse_not()
        while self.try_match("and"):
            right = self.parse_not()
            node = ASTNode("AND", "", [node, right])
        return node

    def parse_not(self):
        if self.try_match("not"):
            expr = self.parse_not()
            return ASTNode("NOT", "", [expr])
        return self.parse_comparison()

    def parse_comparison(self):
        node = self.parse_bitor()
        t = self.peek()
        if t.type == '==' or t.type == '!=' or t.type == '<' or t.type == '>' or t.type == '<=' or t.type == '>=' or t.type == 'in':
            self.pos = self.pos + 1
            right = self.parse_bitor()
            node = ASTNode("COMP", t.type, [node, right])
        return node

    def parse_bitor(self):
        node = self.parse_bitxor()
        while True:
            t = self.peek()
            if t.type == '|':
                self.pos = self.pos + 1
                right = self.parse_bitxor()
                node = ASTNode("BINOP", '|', [node, right])
            else:
                break
        return node

    def parse_bitxor(self):
        node = self.parse_bitand()
        while True:
            t = self.peek()
            if t.type == '^':
                self.pos = self.pos + 1
                right = self.parse_bitand()
                node = ASTNode("BINOP", '^', [node, right])
            else:
                break
        return node

    def parse_bitand(self):
        node = self.parse_shift()
        while True:
            t = self.peek()
            if t.type == '&':
                self.pos = self.pos + 1
                right = self.parse_shift()
                node = ASTNode("BINOP", '&', [node, right])
            else:
                break
        return node

    def parse_shift(self):
        node = self.parse_sum()
        while True:
            t = self.peek()
            if t.type == '<<' or t.type == '>>':
                self.pos = self.pos + 1
                right = self.parse_sum()
                node = ASTNode("BINOP", t.type, [node, right])
            else:
                break
        return node

    def parse_sum(self):
        node = self.parse_term()
        while True:
            t = self.peek()
            if t.type == '+' or t.type == '-':
                self.pos = self.pos + 1
                right = self.parse_term()
                node = ASTNode("BINOP", t.type, [node, right])
            else:
                break
        return node

    def parse_term(self):
        node = self.parse_factor()
        while True:
            t = self.peek()
            if t.type == '*' or t.type == '/' or t.type == '%' or t.type == '//' or t.type == '@':
                self.pos = self.pos + 1
                right = self.parse_factor()
                node = ASTNode("BINOP", t.type, [node, right])
            else:
                break
        return node

    def parse_factor(self):
        t = self.peek()
        if t.type == '+' or t.type == '-' or t.type == '~':
            self.pos = self.pos + 1
            factor = self.parse_factor()
            return ASTNode("UNARY", t.type, [factor])
        return self.parse_power()

    def parse_power(self):
        node = self.parse_primary()
        if self.try_match('**'):
            right = self.parse_power()
            node = ASTNode("BINOP", '**', [node, right])
        return node

    def parse_primary(self):
        node = self.parse_atom()
        while True:
            t = self.peek()
            if t.type == '(':
                self.pos = self.pos + 1
                if self.peek().type != ')':
                    first_arg = self.parse_expr()
                    if self.peek().type == "for":
                        gen = self.parse_listcomp_generators()
                        self.consume(')')
                        node = ASTNode("CALL", "", [node, ASTNode("LIST", "", [ASTNode("GENCOMP", "", [first_arg, gen])])])
                    else:
                        args = [first_arg]
                        while self.try_match(','):
                            args.append(self.parse_expr())
                        self.consume(')')
                        node = ASTNode("CALL", "", [node, ASTNode("LIST", "", args)])
                else:
                    self.consume(')')
                    node = ASTNode("CALL", "", [node, ASTNode("LIST", "", [])])
            elif t.type == '.':
                self.pos = self.pos + 1
                attr = self.consume("NAME").value
                node = ASTNode("ATTR", attr, [node])
            elif t.type == '[':
                self.pos = self.pos + 1
                is_slice = False
                start = None
                end = None
                if self.try_match(':'):
                    is_slice = True
                    start = ASTNode("CONST_NONE", "", [])
                    if self.peek().type != ']':
                        end = self.parse_expr()
                    else:
                        end = ASTNode("CONST_NONE", "", [])
                else:
                    first = self.parse_expr()
                    if self.try_match(':'):
                        is_slice = True
                        start = first
                        if self.peek().type != ']':
                            end = self.parse_expr()
                        else:
                            end = ASTNode("CONST_NONE", "", [])
                    else:
                        node = ASTNode("SUBSCRIPT", "", [node, first])
                if is_slice:
                    node = ASTNode("SLICE", "", [node, start, end])
                self.consume(']')
            else:
                break
        return node

    def parse_atom(self):
        t = self.peek()
        if t.type == "NUMBER":
            self.pos = self.pos + 1
            return ASTNode("CONST_INT", t.value, [])
        elif t.type == "FLOAT":
            self.pos = self.pos + 1
            return ASTNode("CONST_FLOAT", t.value, [])
        elif t.type == "IMAG":
            self.pos = self.pos + 1
            return ASTNode("CONST_IMAG", t.value, [])
        elif t.type == "BYTES":
            self.pos = self.pos + 1
            return ASTNode("CONST_BYTES", t.value, [])
        elif t.type == "STRING":
            self.pos = self.pos + 1
            return ASTNode("CONST_STR", t.value, [])
        elif t.type == "None":
            self.pos = self.pos + 1
            return ASTNode("CONST_NONE", "", [])
        elif t.type == "True":
            self.pos = self.pos + 1
            return ASTNode("CONST_BOOL", "True", [])
        elif t.type == "False":
            self.pos = self.pos + 1
            return ASTNode("CONST_BOOL", "False", [])
        elif t.type == "NAME":
            self.pos = self.pos + 1
            return ASTNode("NAME", t.value, [])
        elif t.type == '(':
            self.pos = self.pos + 1
            if self.peek().type == ')':
                self.consume(')')
                return ASTNode("TUPLE", "", [])
            first = self.parse_expr()
            if self.peek().type == "for":
                gen = self.parse_listcomp_generators()
                self.consume(')')
                return ASTNode("GENCOMP", "", [first, gen])
            if self.peek().type == ',':
                elements = [first]
                while self.try_match(','):
                    if self.peek().type == ')':
                        break
                    elements.append(self.parse_expr())
                self.consume(')')
                return ASTNode("TUPLE", "", elements)
            self.consume(')')
            return first
        elif t.type == '[':
            self.pos = self.pos + 1
            if self.peek().type != ']':
                first_expr = self.parse_expr()
                if self.peek().type == "for":
                    gen = self.parse_listcomp_generators()
                    self.consume(']')
                    return ASTNode("LISTCOMP", "", [first_expr, gen])
                elements = [first_expr]
                while self.try_match(','):
                    elements.append(self.parse_expr())
            else:
                elements = []
            self.consume(']')
            return ASTNode("LIST", "", elements)
        elif t.type == '{':
            self.pos = self.pos + 1
            if self.peek().type == '}':
                self.consume('}')
                return ASTNode("DICT", "", [ASTNode("LIST", "", []), ASTNode("LIST", "", [])])
            first = self.parse_expr()
            if self.peek().type == ':':
                # Dict literal or dict comprehension
                self.pos = self.pos + 1
                value = self.parse_expr()
                if self.peek().type == "for":
                    gen = self.parse_listcomp_generators()
                    self.consume('}')
                    return ASTNode("DICTCOMP", "", [first, value, gen])
                keys = [first]
                values = [value]
                while self.try_match(','):
                    key = self.parse_expr()
                    self.consume(':')
                    val = self.parse_expr()
                    keys.append(key)
                    values.append(val)
                self.consume('}')
                return ASTNode("DICT", "", [ASTNode("LIST", "", keys), ASTNode("LIST", "", values)])
            else:
                # Set literal or set comprehension
                if self.peek().type == "for":
                    gen = self.parse_listcomp_generators()
                    self.consume('}')
                    return ASTNode("SETCOMP", "", [first, gen])
                elements = [first]
                while self.try_match(','):
                    if self.peek().type == '}':
                        break
                    elements.append(self.parse_expr())
                self.consume('}')
                return ASTNode("SET", "", elements)
        elif t.type == "lambda":
            self.pos = self.pos + 1
            params = []
            if self.peek().type != ':':
                params.append(self.consume("NAME").value)
                while self.try_match(','):
                    params.append(self.consume("NAME").value)
            self.consume(':')
            body = self.parse_expr()
            params_node = ASTNode("PARAMS", "", [])
            p_idx = 0
            while p_idx < len(params):
                params_node.children.append(ASTNode("NAME", params[p_idx], []))
                p_idx = p_idx + 1
            return ASTNode("LAMBDA", "", [params_node, body])
        elif t.type == "yield":
            self.pos = self.pos + 1
            value = None
            if self.peek().type == "from":
                self.pos = self.pos + 1
                value = self.parse_expr()
                return ASTNode("YIELD_FROM", "", [value])
            elif self.peek().type != 'NEWLINE' and self.peek().type != ')' and self.peek().type != ',':
                value = self.parse_expr()
            else:
                value = ASTNode("CONST_NONE", "", [])
            return ASTNode("YIELD", "", [value])
        elif t.type == "await":
            self.pos = self.pos + 1
            value = self.parse_expr()
            return ASTNode("AWAIT", "", [value])
        else:
            print("Parser error: unexpected token in expression " + t.type + " at line " + str(t.line))
            self.pos = self.pos + 1
            return ASTNode("CONST_NONE", "", [])

    def parse_suite(self):
        self.consume(':')
        self.consume('NEWLINE')
        self.consume('INDENT')
        stmts = []
        while self.peek().type != 'DEDENT' and self.peek().type != 'EOF':
            stmts.append(self.parse_stmt())
        self.consume('DEDENT')
        return ASTNode("BLOCK", "", stmts)

    def parse_pattern(self):
        t = self.peek()
        if t.type == "NAME" and t.value == "_":
            self.pos = self.pos + 1
            return ASTNode("PATTERN_WC", "", [])
        elif t.type == "NAME":
            self.pos = self.pos + 1
            return ASTNode("PATTERN_CAP", t.value, [])
        else:
            expr = self.parse_expr()
            return ASTNode("PATTERN_LIT", "", [expr])

    def parse_listcomp_generators(self):
        self.consume("for")
        targets = [self.consume("NAME").value]
        while self.peek().type == ',':
            self.pos = self.pos + 1
            targets.append(self.consume("NAME").value)
        var_name = ",".join(targets)
        self.consume("in")
        iterable = self.parse_and()
        if_cond = ASTNode("CONST_NONE", "", [])
        if self.peek().type == "if":
            self.pos = self.pos + 1
            if_cond = self.parse_and()
            while self.peek().type == "if":
                self.pos = self.pos + 1
                next_cond = self.parse_and()
                if_cond = ASTNode("AND", "", [if_cond, next_cond])
        next_gen = ASTNode("CONST_NONE", "", [])
        if self.peek().type == "for":
            next_gen = self.parse_listcomp_generators()
        return ASTNode("GENERATOR", var_name, [iterable, if_cond, next_gen])

    def parse_stmt(self):
        t = self.peek()
        
        # Decorator support
        if t.type == '@':
            self.pos = self.pos + 1
            decorator = self.parse_expr()
            self.consume('NEWLINE')
            decorated = self.parse_stmt()
            if decorated.type == "DEF" or decorated.type == "ASYNC_DEF" or decorated.type == "CLASS":
                decorated.children.append(ASTNode("DECORATOR", "", [decorator]))
            return decorated
        
        if t.type == "def":
            self.pos = self.pos + 1
            name = self.consume("NAME").value
            self.consume('(')
            params = []
            if self.peek().type != ')':
                p_name = self.consume("NAME").value
                # Skip type annotation
                if self.peek().type == ':':
                    self.pos = self.pos + 1
                    self.parse_expr()
                params.append(p_name)
                while self.try_match(','):
                    p_name = self.consume("NAME").value
                    if self.peek().type == ':':
                        self.pos = self.pos + 1
                        self.parse_expr()
                    params.append(p_name)
            self.consume(')')
            # Skip return type annotation
            if self.peek().type == '->':
                self.pos = self.pos + 1
                self.parse_expr()
            body = self.parse_suite()
            params_node = ASTNode("PARAMS", "", [])
            p_idx = 0
            while p_idx < len(params):
                params_node.children.append(ASTNode("NAME", params[p_idx], []))
                p_idx = p_idx + 1
            return ASTNode("DEF", name, [params_node, body])

        elif t.type == "class":
            self.pos = self.pos + 1
            name = self.consume("NAME").value
            # Optional inheritance
            if self.try_match('('):
                base_name = self.consume("NAME").value
                self.consume(')')
            body = self.parse_suite()
            return ASTNode("CLASS", name, [body])

        elif t.type == "if":
            self.pos = self.pos + 1
            cond = self.parse_expr()
            then_branch = self.parse_suite()
            
            curr_node = ASTNode("IF", "", [cond, then_branch])
            parent_node = curr_node
            
            while True:
                next_t = self.peek()
                if next_t.type == "elif":
                    self.pos = self.pos + 1
                    elif_cond = self.parse_expr()
                    elif_body = self.parse_suite()
                    elif_node = ASTNode("IF", "", [elif_cond, elif_body])
                    parent_node.children.append(elif_node)
                    parent_node = elif_node
                elif next_t.type == "else":
                    self.pos = self.pos + 1
                    else_body = self.parse_suite()
                    parent_node.children.append(else_body)
                    break
                else:
                    break
            return curr_node

        elif t.type == "while":
            self.pos = self.pos + 1
            cond = self.parse_expr()
            body = self.parse_suite()
            if self.try_match("else"):
                else_body = self.parse_suite()
                return ASTNode("WHILE", "", [cond, body, else_body])
            return ASTNode("WHILE", "", [cond, body])

        elif t.type == "for":
            self.pos = self.pos + 1
            var_name = self.consume("NAME").value
            self.consume("in")
            iterable = self.parse_expr()
            body = self.parse_suite()
            if self.try_match("else"):
                else_body = self.parse_suite()
                return ASTNode("FOR", var_name, [iterable, body, else_body])
            return ASTNode("FOR", var_name, [iterable, body])

        elif t.type == "return":
            self.pos = self.pos + 1
            value = None
            if self.peek().type != 'NEWLINE':
                value = self.parse_expr()
            else:
                value = ASTNode("CONST_NONE", "", [])
            self.consume('NEWLINE')
            return ASTNode("RETURN", "", [value])

        elif t.type == "pass":
            self.pos = self.pos + 1
            self.consume('NEWLINE')
            return ASTNode("PASS", "", [])

        elif t.type == "break":
            self.pos = self.pos + 1
            self.consume('NEWLINE')
            return ASTNode("BREAK", "", [])

        elif t.type == "continue":
            self.pos = self.pos + 1
            self.consume('NEWLINE')
            return ASTNode("CONTINUE", "", [])

        elif t.type == "import":
            self.pos = self.pos + 1
            mod_parts = self.consume("NAME").value
            while self.peek().type == '.':
                self.pos = self.pos + 1
                mod_parts = mod_parts + '.' + self.consume("NAME").value
            alias = ASTNode("CONST_NONE", "", [])
            if self.try_match("as"):
                alias = ASTNode("NAME", self.consume("NAME").value, [])
            if self.peek().type == ',':
                modules = [ASTNode("IMPORT", mod_parts, [alias])]
                while self.try_match(','):
                    mod_parts = self.consume("NAME").value
                    while self.peek().type == '.':
                        self.pos = self.pos + 1
                        mod_parts = mod_parts + '.' + self.consume("NAME").value
                    alias = ASTNode("CONST_NONE", "", [])
                    if self.try_match("as"):
                        alias = ASTNode("NAME", self.consume("NAME").value, [])
                    modules.append(ASTNode("IMPORT", mod_parts, [alias]))
                self.consume('NEWLINE')
                return ASTNode("IMPORT_LIST", "", modules)
            self.consume('NEWLINE')
            return ASTNode("IMPORT", mod_parts, [alias])

        elif t.type == "from":
            self.pos = self.pos + 1
            mod_parts = self.consume("NAME").value
            while self.peek().type == '.':
                self.pos = self.pos + 1
                mod_parts = mod_parts + '.' + self.consume("NAME").value
            self.consume("import")
            imported = []
            while True:
                name = self.consume("NAME").value
                alias = ASTNode("CONST_NONE", "", [])
                if self.try_match("as"):
                    alias = ASTNode("NAME", self.consume("NAME").value, [])
                if alias.type == "CONST_NONE":
                    imported.append(ASTNode("NAME", name, []))
                else:
                    imported.append(ASTNode("IMPORT_AS", "", [ASTNode("NAME", name, []), alias]))
                if not self.try_match(','):
                    break
            self.consume('NEWLINE')
            return ASTNode("FROM_IMPORT", mod_parts, imported)

        elif t.type == "raise":
            self.pos = self.pos + 1
            value = None
            if self.peek().type != 'NEWLINE':
                value = self.parse_expr()
            else:
                value = ASTNode("CONST_NONE", "", [])
            self.consume('NEWLINE')
            return ASTNode("RAISE", "", [value])

        elif t.type == "assert":
            self.pos = self.pos + 1
            test = self.parse_expr()
            msg = ASTNode("CONST_NONE", "", [])
            if self.peek().type != 'NEWLINE':
                self.consume(',')
                msg = self.parse_expr()
            self.consume('NEWLINE')
            return ASTNode("ASSERT", "", [test, msg])

        elif t.type == "del":
            self.pos = self.pos + 1
            target = self.parse_expr()
            self.consume('NEWLINE')
            return ASTNode("DEL", "", [target])

        elif t.type == "global":
            self.pos = self.pos + 1
            names = []
            names.append(self.consume("NAME").value)
            while self.try_match(','):
                names.append(self.consume("NAME").value)
            self.consume('NEWLINE')
            names_node = ASTNode("LIST", "", [])
            n_idx = 0
            while n_idx < len(names):
                names_node.children.append(ASTNode("NAME", names[n_idx], []))
                n_idx = n_idx + 1
            return ASTNode("GLOBAL", "", [names_node])

        elif t.type == "nonlocal":
            self.pos = self.pos + 1
            names = []
            names.append(self.consume("NAME").value)
            while self.try_match(','):
                names.append(self.consume("NAME").value)
            self.consume('NEWLINE')
            names_node = ASTNode("LIST", "", [])
            n_idx = 0
            while n_idx < len(names):
                names_node.children.append(ASTNode("NAME", names[n_idx], []))
                n_idx = n_idx + 1
            return ASTNode("NONLOCAL", "", [names_node])

        elif t.type == "with":
            self.pos = self.pos + 1
            expr = self.parse_expr()
            alias = None
            if self.try_match("as"):
                alias = self.consume("NAME").value
            body = self.parse_suite()
            alias_node = ASTNode("CONST_NONE", "", [])
            if alias != None:
                alias_node = ASTNode("NAME", alias, [])
            return ASTNode("WITH", "", [expr, alias_node, body])

        elif t.type == "try":
            self.pos = self.pos + 1
            body = self.parse_suite()
            handlers = []
            else_body = ASTNode("CONST_NONE", "", [])
            finally_body = ASTNode("CONST_NONE", "", [])

            while True:
                next_t = self.peek()
                if next_t.type == "except":
                    self.pos = self.pos + 1
                    exc_type = ASTNode("CONST_NONE", "", [])
                    exc_var = None
                    if self.peek().type != ':':
                        exc_type = self.parse_expr()
                        if self.try_match("as"):
                            exc_var = self.consume("NAME").value
                    handler_body = self.parse_suite()
                    handlers.append(ASTNode("EXCEPT", exc_var or "", [exc_type, handler_body]))
                elif next_t.type == "else":
                    self.pos = self.pos + 1
                    else_body = self.parse_suite()
                elif next_t.type == "finally":
                    self.pos = self.pos + 1
                    finally_body = self.parse_suite()
                    break
                else:
                    break

            handlers_node = ASTNode("HANDLERS", "", handlers)
            return ASTNode("TRY", "", [body, handlers_node, else_body, finally_body])

        elif t.type == "async":
            self.pos = self.pos + 1
            next_t = self.peek()
            if next_t.type == "def":
                self.pos = self.pos + 1
                name = self.consume("NAME").value
                self.consume('(')
                params = []
                if self.peek().type != ')':
                    p_name = self.consume("NAME").value
                    if self.peek().type == ':':
                        self.pos = self.pos + 1
                        self.parse_expr()
                    params.append(p_name)
                    while self.try_match(','):
                        p_name = self.consume("NAME").value
                        if self.peek().type == ':':
                            self.pos = self.pos + 1
                            self.parse_expr()
                        params.append(p_name)
                self.consume(')')
                if self.peek().type == '->':
                    self.pos = self.pos + 1
                    self.parse_expr()
                body = self.parse_suite()
                params_node = ASTNode("PARAMS", "", [])
                p_idx = 0
                while p_idx < len(params):
                    params_node.children.append(ASTNode("NAME", params[p_idx], []))
                    p_idx = p_idx + 1
                return ASTNode("ASYNC_DEF", name, [params_node, body])
            elif next_t.type == "for":
                self.pos = self.pos + 1
                var_name = self.consume("NAME").value
                self.consume("in")
                iterable = self.parse_expr()
                body = self.parse_suite()
                if self.try_match("else"):
                    else_body = self.parse_suite()
                    return ASTNode("ASYNC_FOR", var_name, [iterable, body, else_body])
                return ASTNode("ASYNC_FOR", var_name, [iterable, body])
            elif next_t.type == "with":
                self.pos = self.pos + 1
                expr = self.parse_expr()
                alias = None
                if self.try_match("as"):
                    alias = self.consume("NAME").value
                body = self.parse_suite()
                alias_node = ASTNode("CONST_NONE", "", [])
                if alias != None:
                    alias_node = ASTNode("NAME", alias, [])
                return ASTNode("ASYNC_WITH", "", [expr, alias_node, body])
            else:
                print("Parser error: expected def/for/with after async at line " + str(t.line))
                self.consume('NEWLINE')
                return ASTNode("PASS", "", [])

        elif t.type == "match":
            self.pos = self.pos + 1
            subject = self.parse_expr()
            self.consume(':')
            self.consume('NEWLINE')
            self.consume('INDENT')
            cases = []
            while self.peek().type == "case":
                self.pos = self.pos + 1
                pattern = self.parse_pattern()
                self.consume(':')
                self.consume('NEWLINE')
                self.consume('INDENT')
                body_stmts = []
                while self.peek().type != 'DEDENT' and self.peek().type != 'EOF':
                    body_stmts.append(self.parse_stmt())
                self.consume('DEDENT')
                body = ASTNode("BLOCK", "", body_stmts)
                cases.append(ASTNode("CASE", "", [pattern, body]))
            self.consume('DEDENT')
            result = ASTNode("MATCH", "", [subject])
            result.children.extend(cases)
            return result

        else:
            expr = self.parse_expr()
            next_t = self.peek()
            if next_t.type == ':=':
                self.pos = self.pos + 1
                value = self.parse_expr()
                self.consume('NEWLINE')
                return ASTNode("ASSIGN", "", [expr, value])
            elif next_t.type == '=':
                self.pos = self.pos + 1
                value = self.parse_expr()
                self.consume('NEWLINE')
                return ASTNode("ASSIGN", "", [expr, value])
            elif next_t.type == '+=' or next_t.type == '-=' or next_t.type == '*=' or next_t.type == '/=' or next_t.type == '%=' or next_t.type == '**=' or next_t.type == '//=' or next_t.type == '&=' or next_t.type == '|=' or next_t.type == '^=' or next_t.type == '>>=' or next_t.type == '<<=' or next_t.type == '@=':
                op = next_t.type
                self.pos = self.pos + 1
                value = self.parse_expr()
                self.consume('NEWLINE')
                return ASTNode("AUGASSIGN", op, [expr, value])
            else:
                self.consume('NEWLINE')
                return ASTNode("EXPR", "", [expr])

    def parse_module(self):
        stmts = []
        while self.peek().type != 'EOF':
            # Skip stray newlines at the module level
            if self.try_match('NEWLINE'):
                continue
            stmts.append(self.parse_stmt())
        return ASTNode("BLOCK", "", stmts)

def collect_locals(node, locals_list):
    if node.type == "ASSIGN" or node.type == "AUGASSIGN":
        target = node.children[0]
        if target.type == "NAME":
            name = target.value
            found = False
            i = 0
            while i < len(locals_list):
                if locals_list[i] == name:
                    found = True
                    break
                i = i + 1
            if not found:
                locals_list.append(name)
    elif node.type == "FOR" or node.type == "ASYNC_FOR":
        name = node.value
        found = False
        i = 0
        while i < len(locals_list):
            if locals_list[i] == name:
                found = True
                break
            i = i + 1
        if not found:
            locals_list.append(name)
    
    
    if node.type == "EXCEPT":
        exc_var = node.value
        if exc_var != "":
            found = False
            i = 0
            while i < len(locals_list):
                if locals_list[i] == exc_var:
                    found = True
                    break
                i = i + 1
            if not found:
                locals_list.append(exc_var)

    if node.type == "WITH" or node.type == "ASYNC_WITH":
        alias_node = node.children[1]
        if alias_node.type == "NAME":
            name = alias_node.value
            found = False
            i = 0
            while i < len(locals_list):
                if locals_list[i] == name:
                    found = True
                    break
                i = i + 1
            if not found:
                locals_list.append(name)
    
    if node.type == "PATTERN_CAP":
        name = node.value
        found = False
        i = 0
        while i < len(locals_list):
            if locals_list[i] == name:
                found = True
                break
            i = i + 1
        if not found:
            locals_list.append(name)

    if node.type == "IMPORT":
        name = node.value.split('.')[0]
        alias_node = node.children[0]
        if alias_node.type == "NAME":
            name = alias_node.value
        found = False
        i = 0
        while i < len(locals_list):
            if locals_list[i] == name:
                found = True
                break
            i = i + 1
        if not found:
            locals_list.append(name)
        return

    if node.type == "FROM_IMPORT":
        imported = node.children
        i_idx = 0
        while i_idx < len(imported):
            item = imported[i_idx]
            if item.type == "NAME":
                name = item.value
            elif item.type == "IMPORT_AS":
                name = item.children[1].value
            found = False
            i = 0
            while i < len(locals_list):
                if locals_list[i] == name:
                    found = True
                    break
                i = i + 1
            if not found:
                locals_list.append(name)
            i_idx = i_idx + 1
        return

    if node.type == "DEF" or node.type == "ASYNC_DEF" or node.type == "LAMBDA":
        return

    idx = 0
    while idx < len(node.children):
        collect_locals(node.children[idx], locals_list)
        idx = idx + 1

def collect_globals(block_node, globals_list):
    stmts = block_node.children
    idx = 0
    while idx < len(stmts):
        node = stmts[idx]
        collect_locals(node, globals_list)
        idx = idx + 1

def scan_global_nonlocal_names(body_node):
    names = []
    stmts = body_node.children
    idx = 0
    while idx < len(stmts):
        stmt = stmts[idx]
        if stmt.type == "GLOBAL" or stmt.type == "NONLOCAL":
            list_node = stmt.children[0]
            n_idx = 0
            while n_idx < len(list_node.children):
                name_node = list_node.children[n_idx]
                if name_node.type == "NAME":
                    names.append(name_node.value)
                n_idx = n_idx + 1
        idx = idx + 1
    return names

def has_yield(node):
    if node.type == "YIELD" or node.type == "YIELD_FROM":
        return True
    if node.type == "DEF" or node.type == "ASYNC_DEF" or node.type == "CLASS":
        return False
    i = 0
    while i < len(node.children):
        if has_yield(node.children[i]):
            return True
        i = i + 1
    return False

def escape_c_string(s):
    res = ""
    i = 0
    while i < len(s):
        c = s[i]
        if c == '\n':
            res = res + "\\n"
        elif c == '\t':
            res = res + "\\t"
        elif c == '\r':
            res = res + "\\r"
        elif c == '\\':
            res = res + "\\\\"
        elif c == '"':
            res = res + "\\\""
        else:
            res = res + c
        i = i + 1
    return res

stdlib_list = ["sys", "os", "math", "json", "re", "time", "datetime", "random", "collections", "itertools", "functools", "pathlib", "subprocess", "typing"]

def is_stdlib_module(name):
    s_idx = 0
    while s_idx < len(stdlib_list):
        if stdlib_list[s_idx] == name:
            return True
        s_idx = s_idx + 1
    return False

def try_open_file(path):
    try:
        f = open(path, "r")
        f.close()
        return True
    except:
        return False

def find_module_file(module_name, input_dir):
    parts = module_name.split('.')
    if input_dir == "" or input_dir == ".":
        base_path = ""
    else:
        base_path = input_dir
    if base_path != "":
        if base_path[len(base_path)-1] != '/':
            base_path = base_path + '/'
    f_idx = 0
    while f_idx < len(parts):
        base_path = base_path + parts[f_idx] + '/'
        f_idx = f_idx + 1
    if base_path[len(base_path)-1] == '/':
        base_path = base_path[0:len(base_path)-1]
    candidates = base_path + ".py"
    if try_open_file(candidates):
        return candidates
    candidates = base_path + ".turbo"
    if try_open_file(candidates):
        return candidates
    return None

def find_package_init(module_name, input_dir):
    parts = module_name.split('.')
    base_path = input_dir
    if base_path == "" or base_path == ".":
        base_path = ""
    if base_path != "":
        if base_path[len(base_path)-1] != '/':
            base_path = base_path + '/'
    p_idx = 0
    while p_idx < len(parts):
        base_path = base_path + parts[p_idx] + '/'
        p_idx = p_idx + 1
    candidates = base_path + "__init__.py"
    if try_open_file(candidates):
        return candidates
    candidates = base_path + "__init__.turbo"
    if try_open_file(candidates):
        return candidates
    return None

class CodeGen:
    def __init__(self):
        self.header = ""
        self.funcs = ""
        self.main_body = ""
        self.indent_level = 0
        self.local_vars = []
        self.else_loop_stack = []
        self.else_loop_counter = 0
        self.is_generator = False
        self.lambda_counter = 0
        self.processed_modules = []
        self.input_dir = "."
        self.builtins_list = ["print", "len", "str", "int", "ord", "chr", "range", "open", "sys_argv", "input", "type", "isinstance", "hasattr", "getattr", "setattr", "repr", "abs", "round", "pow", "hex", "bin", "oct", "float", "bool", "list", "dict", "super", "iter", "next", "all", "any", "sum", "min", "max", "sorted", "reversed", "enumerate", "zip", "map", "filter"]

    def write_header(self, s):
        self.header = self.header + s

    def write_func(self, s):
        self.funcs = self.funcs + s

    def write_main(self, s):
        i = 0
        while i < self.indent_level:
            self.main_body = self.main_body + "    "
            i = i + 1
        self.main_body = self.main_body + s + "\n"

    def write_code(self, s, is_in_func):
        if is_in_func:
            i = 0
            while i < self.indent_level:
                self.funcs = self.funcs + "    "
                i = i + 1
            self.funcs = self.funcs + s + "\n"
        else:
            self.write_main(s)

    def gen_expr(self, node):
        if node.type == "CONST_INT":
            return 'make_int("' + node.value + '")'
        elif node.type == "CONST_FLOAT":
            return "make_float(" + node.value + ")"
        elif node.type == "CONST_IMAG":
            imag_val = node.value
            if imag_val[len(imag_val)-1] == 'j' or imag_val[len(imag_val)-1] == 'J':
                imag_val = imag_val[0:len(imag_val)-1]
            if imag_val == "":
                imag_val = "1"
            return "make_complex(0.0, " + imag_val + ")"
        elif node.type == "CONST_BYTES":
            return 'make_bytes_from_str("' + escape_c_string(node.value) + '")'
        elif node.type == "CONST_STR":
            return 'make_str("' + escape_c_string(node.value) + '")'
        elif node.type == "CONST_NONE":
            return "turbo_none"
        elif node.type == "CONST_BOOL":
            if node.value == "True":
                return "turbo_true"
            else:
                return "turbo_false"
        elif node.type == "NAME":
            return "t_" + node.value
        elif node.type == "BINOP":
            left = self.gen_expr(node.children[0])
            right = self.gen_expr(node.children[1])
            op = node.value
            if op == '+':
                return "turbo_add(" + left + ", " + right + ")"
            elif op == '-':
                return "turbo_sub(" + left + ", " + right + ")"
            elif op == '*':
                return "turbo_mul(" + left + ", " + right + ")"
            elif op == '/':
                return "turbo_div(" + left + ", " + right + ")"
            elif op == '%':
                return "turbo_mod(" + left + ", " + right + ")"
            elif op == '**':
                return "turbo_pow(" + left + ", " + right + ")"
            elif op == '//':
                return "turbo_floordiv(" + left + ", " + right + ")"
            elif op == '&':
                return "turbo_bitand(" + left + ", " + right + ")"
            elif op == '|':
                return "turbo_bitor(" + left + ", " + right + ")"
            elif op == '^':
                return "turbo_bitxor(" + left + ", " + right + ")"
            elif op == '<<':
                return "turbo_lshift(" + left + ", " + right + ")"
            elif op == '>>':
                return "turbo_rshift(" + left + ", " + right + ")"
            elif op == '@':
                return "turbo_matmul(" + left + ", " + right + ")"
        elif node.type == "UNARY":
            operand = self.gen_expr(node.children[0])
            op = node.value
            if op == '-':
                return 'turbo_sub(make_int("0"), ' + operand + ")"
            elif op == '+':
                return operand
            elif op == '~':
                return "turbo_bitnot(" + operand + ")"
        elif node.type == "COMP":
            left = self.gen_expr(node.children[0])
            right = self.gen_expr(node.children[1])
            op = node.value
            if op == '==':
                return "turbo_eq(" + left + ", " + right + ")"
            elif op == '!=':
                return "turbo_ne(" + left + ", " + right + ")"
            elif op == '<':
                return "turbo_lt(" + left + ", " + right + ")"
            elif op == '>':
                return "turbo_gt(" + left + ", " + right + ")"
            elif op == '<=':
                return "turbo_lte(" + left + ", " + right + ")"
            elif op == '>=':
                return "turbo_gte(" + left + ", " + right + ")"
            elif op == 'in':
                return "make_bool(turbo_in(" + left + ", " + right + "))"
        elif node.type == "NOT":
            operand = self.gen_expr(node.children[0])
            return "turbo_not(" + operand + ")"
        elif node.type == "AND":
            left = self.gen_expr(node.children[0])
            right = self.gen_expr(node.children[1])
            return "({ TurboObject* _lh = " + left + "; turbo_is_truthy(_lh) ? " + right + " : _lh; })"
        elif node.type == "OR":
            left = self.gen_expr(node.children[0])
            right = self.gen_expr(node.children[1])
            return "({ TurboObject* _lh = " + left + "; turbo_is_truthy(_lh) ? _lh : " + right + "; })"
        elif node.type == "CALL":
            func_node = node.children[0]
            args_list = node.children[1].children
            argc = len(args_list)
            args_c = "NULL"
            if argc > 0:
                args_c = "(TurboObject*[]){"
                a_idx = 0
                while a_idx < argc:
                    args_c = args_c + self.gen_expr(args_list[a_idx])
                    if a_idx < argc - 1:
                        args_c = args_c + ", "
                    a_idx = a_idx + 1
                args_c = args_c + "}"

            if func_node.type == "ATTR":
                obj_expr = self.gen_expr(func_node.children[0])
                method_name = func_node.value
                return "turbo_call_method(" + obj_expr + ', "' + method_name + '", ' + str(argc) + ", " + args_c + ")"
            else:
                func_expr = self.gen_expr(func_node)
                return "turbo_call(" + func_expr + ", " + str(argc) + ", " + args_c + ")"
        elif node.type == "ATTR":
            obj_expr = self.gen_expr(node.children[0])
            attr_name = node.value
            return "turbo_getattr(" + obj_expr + ', "' + attr_name + '")'
        elif node.type == "SUBSCRIPT":
            obj_expr = self.gen_expr(node.children[0])
            idx_expr = self.gen_expr(node.children[1])
            return "turbo_getitem(" + obj_expr + ", " + idx_expr + ")"
        elif node.type == "SLICE":
            obj_expr = self.gen_expr(node.children[0])
            start_expr = self.gen_expr(node.children[1])
            end_expr = self.gen_expr(node.children[2])
            return "turbo_slice(" + obj_expr + ", " + start_expr + ", " + end_expr + ")"
        elif node.type == "LIST":
            argc = len(node.children)
            if argc == 0:
                return "make_list()"
            c_code = "({ TurboObject* _lst = make_list(); "
            a_idx = 0
            while a_idx < argc:
                c_code = c_code + "turbo_list_append(_lst, " + self.gen_expr(node.children[a_idx]) + "); "
                a_idx = a_idx + 1
            c_code = c_code + " _lst; })"
            return c_code
        elif node.type == "TUPLE":
            argc = len(node.children)
            if argc == 0:
                return "make_tuple()"
            c_code = "({ TurboObject* _tup = make_tuple(); "
            a_idx = 0
            while a_idx < argc:
                c_code = c_code + "turbo_tuple_append(_tup, " + self.gen_expr(node.children[a_idx]) + "); "
                a_idx = a_idx + 1
            c_code = c_code + " _tup; })"
            return c_code
        elif node.type == "SET":
            argc = len(node.children)
            if argc == 0:
                return "make_set()"
            c_code = "({ TurboObject* _set = make_set(); "
            a_idx = 0
            while a_idx < argc:
                c_code = c_code + "turbo_set_add(_set, " + self.gen_expr(node.children[a_idx]) + "); "
                a_idx = a_idx + 1
            c_code = c_code + " _set; })"
            return c_code
        elif node.type == "SETCOMP":
            elem_c = self.gen_expr(node.children[0])
            gen = node.children[1]
            c_code = "({ TurboObject* _sc = make_set(); "
            c_code = c_code + self._gen_comp_loops(gen, "_sc", "turbo_set_add(_sc, " + elem_c + ")")
            c_code = c_code + " _sc; })"
            return c_code
        elif node.type == "LISTCOMP":
            elem_c = self.gen_expr(node.children[0])
            gen = node.children[1]
            c_code = "({ TurboObject* _lc = make_list(); "
            c_code = c_code + self._gen_comp_loops(gen, "_lc", "turbo_list_append(_lc, " + elem_c + ")")
            c_code = c_code + " _lc; })"
            return c_code
        elif node.type == "DICTCOMP":
            key_c = self.gen_expr(node.children[0])
            val_c = self.gen_expr(node.children[1])
            gen = node.children[2]
            c_code = "({ TurboObject* _dc = make_dict(); "
            c_code = c_code + self._gen_comp_loops(gen, "_dc", "turbo_setitem(_dc, " + key_c + ", " + val_c + ")")
            c_code = c_code + " _dc; })"
            return c_code
        elif node.type == "GENCOMP":
            elem_c = self.gen_expr(node.children[0])
            gen = node.children[1]
            c_code = "({ TurboObject* _gc = make_list(); "
            c_code = c_code + self._gen_comp_loops(gen, "_gc", "turbo_list_append(_gc, " + elem_c + ")")
            c_code = c_code + " _gc; })"
            return c_code
        elif node.type == "DICT":
            keys = node.children[0].children
            values = node.children[1].children
            argc = len(keys)
            if argc == 0:
                return "make_dict()"
            c_code = "({ TurboObject* _d = make_dict(); "
            a_idx = 0
            while a_idx < argc:
                c_code = c_code + "turbo_setitem(_d, " + self.gen_expr(keys[a_idx]) + ", " + self.gen_expr(values[a_idx]) + "); "
                a_idx = a_idx + 1
            c_code = c_code + " _d; })"
            return c_code
        elif node.type == "TERNARY":
            cond = self.gen_expr(node.children[0])
            true_val = self.gen_expr(node.children[1])
            false_val = self.gen_expr(node.children[2])
            return "turbo_is_truthy(" + cond + ") ? " + true_val + " : " + false_val
        elif node.type == "ASSIGN":
            target = node.children[0]
            val_expr = self.gen_expr(node.children[1])
            if target.type == "NAME":
                return "({ TurboObject* _w = " + val_expr + "; t_" + target.value + " = _w; _w; })"
            return val_expr
        elif node.type == "LAMBDA":
            lambda_num = self.lambda_counter
            self.lambda_counter = self.lambda_counter + 1
            impl_name = "t_impl_lambda_" + str(lambda_num)
            params_node = node.children[0]
            body_node = node.children[1]

            old_locals = self.local_vars
            self.local_vars = []
            p_idx = 0
            while p_idx < len(params_node.children):
                self.local_vars.append(params_node.children[p_idx].value)
                p_idx = p_idx + 1
            collect_locals(body_node, self.local_vars)

            self.write_header("TurboObject* " + impl_name + "(int argc, TurboObject** args);\n")

            self.funcs = self.funcs + "TurboObject* " + impl_name + "(int argc, TurboObject** args) {\n"

            p_idx = 0
            while p_idx < len(params_node.children):
                p_name = params_node.children[p_idx].value
                self.funcs = self.funcs + "    TurboObject* t_" + p_name + " = (argc > " + str(p_idx) + ") ? args[" + str(p_idx) + "] : turbo_none;\n"
                p_idx = p_idx + 1

            l_idx = 0
            while l_idx < len(self.local_vars):
                l_name = self.local_vars[l_idx]
                is_param = False
                p_idx = 0
                while p_idx < len(params_node.children):
                    if params_node.children[p_idx].value == l_name:
                        is_param = True
                        break
                    p_idx = p_idx + 1
                if not is_param:
                    self.funcs = self.funcs + "    TurboObject* t_" + l_name + " = turbo_none;\n"
                l_idx = l_idx + 1

            body_code = self.gen_expr(body_node)
            self.funcs = self.funcs + "    return " + body_code + ";\n"
            self.funcs = self.funcs + "}\n\n"

            self.local_vars = old_locals

            return "make_func(" + impl_name + ', "<lambda>")'
        elif node.type == "YIELD":
            val_c = self.gen_expr(node.children[0])
            return "({ TurboObject* __yv = " + val_c + "; turbo_list_append(__yield_values, __yv); turbo_none; })"
        elif node.type == "YIELD_FROM":
            iter_c = self.gen_expr(node.children[0])
            return "({ TurboObject* __sub = " + iter_c + "; if (__sub->type == TYPE_LIST) { for (int __i = 0; __i < __sub->list_val.length; __i++) { turbo_list_append(__yield_values, __sub->list_val.items[__i]); } } else if (__sub->type == TYPE_TUPLE) { for (int __i = 0; __i < __sub->tuple_val.length; __i++) { turbo_list_append(__yield_values, __sub->tuple_val.items[__i]); } } else if (__sub->type == TYPE_STR) { for (int __i = 0; __i < __sub->str_val.length; __i++) { char __tmp[2] = {__sub->str_val.chars[__i], '\\0'}; turbo_list_append(__yield_values, make_str(__tmp)); } } turbo_none; })"
        elif node.type == "AWAIT":
            return self.gen_expr(node.children[0])
        else:
            print("CodeGen error: unknown expression " + node.type)
            return "turbo_none"

    def gen_func_def(self, node, class_name, mod_prefix):
        func_name = node.value
        if class_name != "":
            impl_name = "t_impl_" + class_name + "_" + func_name
        else:
            impl_name = "t_impl_" + mod_prefix + func_name
        
        params_node = node.children[0]
        body_node = node.children[1]
        
        old_locals = self.local_vars
        self.local_vars = []
        p_idx = 0
        while p_idx < len(params_node.children):
            self.local_vars.append(params_node.children[p_idx].value)
            p_idx = p_idx + 1
        collect_locals(body_node, self.local_vars)

        global_names = scan_global_nonlocal_names(body_node)

        if len(global_names) > 0:
            new_locals = []
            l_idx = 0
            while l_idx < len(self.local_vars):
                l_name = self.local_vars[l_idx]
                is_global = False
                n_idx = 0
                while n_idx < len(global_names):
                    if global_names[n_idx] == l_name:
                        is_global = True
                        break
                    n_idx = n_idx + 1
                if not is_global:
                    new_locals.append(l_name)
                l_idx = l_idx + 1
            self.local_vars = new_locals
        
        generator_func = has_yield(body_node)
        old_generator = self.is_generator
        self.is_generator = generator_func

        self.write_header("TurboObject* " + impl_name + "(int argc, TurboObject** args);\n")
        
        self.funcs = self.funcs + "TurboObject* " + impl_name + "(int argc, TurboObject** args) {\n"
        
        p_idx = 0
        while p_idx < len(params_node.children):
            p_name = params_node.children[p_idx].value
            self.funcs = self.funcs + "    TurboObject* t_" + p_name + " = (argc > " + str(p_idx) + ") ? args[" + str(p_idx) + "] : turbo_none;\n"
            p_idx = p_idx + 1
            
        l_idx = 0
        while l_idx < len(self.local_vars):
            l_name = self.local_vars[l_idx]
            is_param = False
            p_idx = 0
            while p_idx < len(params_node.children):
                if params_node.children[p_idx].value == l_name:
                    is_param = True
                    break
                p_idx = p_idx + 1
            if not is_param:
                self.funcs = self.funcs + "    TurboObject* t_" + l_name + " = turbo_none;\n"
            l_idx = l_idx + 1
        
        if generator_func:
            self.funcs = self.funcs + "    TurboObject* __yield_values = make_list();\n"
            
        self.indent_level = 1
        self.gen_block_stmts(body_node, True)
        
        if generator_func:
            self.funcs = self.funcs + "    return __yield_values;\n"
        else:
            self.funcs = self.funcs + "    return turbo_none;\n"
        self.funcs = self.funcs + "}\n\n"
        
        self.is_generator = old_generator
        
        self.local_vars = old_locals
        
        if class_name == "":
            self.write_header("TurboObject* t_" + mod_prefix + func_name + " = NULL;\n")
            if mod_prefix == "":
                self.write_main("t_" + func_name + " = make_func(t_impl_" + func_name + ', "' + func_name + '");')
                d_idx = 2
                while d_idx < len(node.children):
                    if node.children[d_idx].type == "DECORATOR":
                        decorator_c = self.gen_expr(node.children[d_idx].children[0])
                        self.write_main("t_" + func_name + " = turbo_call(" + decorator_c + ", 1, (TurboObject*[]){t_" + func_name + "});")
                    d_idx = d_idx + 1

    def gen_class_def(self, node, mod_prefix):
        class_name = node.value
        full_name = mod_prefix + class_name
        self.write_header("TurboObject* t_" + full_name + " = NULL;\n")
        if mod_prefix == "":
            self.write_main("t_" + class_name + ' = make_class("' + class_name + '");')
        
        suite = node.children[0]
        m_idx = 0
        while m_idx < len(suite.children):
            m_node = suite.children[m_idx]
            if m_node.type == "DEF" or m_node.type == "ASYNC_DEF":
                self.gen_func_def(m_node, full_name, "")
                method_name = m_node.value
                if mod_prefix == "":
                    self.write_main("turbo_class_add_method(t_" + class_name + ', "' + method_name + '", t_impl_' + class_name + '_' + method_name + ');')
            m_idx = m_idx + 1
        
        d_idx = 1
        while d_idx < len(node.children):
            if node.children[d_idx].type == "DECORATOR":
                decorator_c = self.gen_expr(node.children[d_idx].children[0])
                if mod_prefix == "":
                    self.write_main("t_" + class_name + " = turbo_call(" + decorator_c + ", 1, (TurboObject*[]){t_" + class_name + "});")
            d_idx = d_idx + 1

    def _gen_comp_loops(self, gen_node, container, append_code):
        if gen_node.type == "CONST_NONE":
            return append_code + "; "
        targets = gen_node.value.split(',')
        iter_c = self.gen_expr(gen_node.children[0])
        if_cond = gen_node.children[1]
        if_c = ""
        if if_cond.type != "CONST_NONE":
            if_c = "if (turbo_is_truthy(" + self.gen_expr(if_cond) + ")) "
        next_gen = gen_node.children[2]
        inner = ""
        if next_gen.type != "CONST_NONE":
            inner = self._gen_comp_loops(next_gen, container, append_code)
        else:
            inner = append_code + "; "
        if len(targets) == 1:
            sv = "TurboObject* t_" + gen_node.value + " = _lci->list_val.items[_lci_i];"
            svt = "TurboObject* t_" + gen_node.value + " = _lci->tuple_val.items[_lci_i];"
            svs = "char _lci_tmp[2] = {_lci->str_val.chars[_lci_i], '\\0'}; TurboObject* t_" + gen_node.value + " = make_str(_lci_tmp);"
        else:
            sv = "TurboObject* _lc_tmp = _lci->list_val.items[_lci_i]; "
            svt = "TurboObject* _lc_tmp = _lci->tuple_val.items[_lci_i]; "
            svs = ""
            i = 0
            while i < len(targets):
                t = targets[i]
                sv += "TurboObject* t_" + t + " = turbo_getitem(_lc_tmp, make_int_from_ll(" + str(i) + ")); "
                svt += "TurboObject* t_" + t + " = turbo_getitem(_lc_tmp, make_int_from_ll(" + str(i) + ")); "
                i = i + 1
        code = "{ TurboObject* _lci = " + iter_c + "; "
        code = code + "if (_lci->type == TYPE_LIST) { for (int _lci_i = 0; _lci_i < _lci->list_val.length; _lci_i++) { " + sv + if_c + "{ " + inner + " } } } "
        code = code + "else if (_lci->type == TYPE_TUPLE) { for (int _lci_i = 0; _lci_i < _lci->tuple_val.length; _lci_i++) { " + svt + if_c + "{ " + inner + " } } } "
        if len(targets) == 1:
            code = code + "else if (_lci->type == TYPE_STR) { for (int _lci_i = 0; _lci_i < _lci->str_val.length; _lci_i++) { " + svs + if_c + "{ " + inner + " } } } "
        code = code + "} "
        return code

    def _load_module(self, module_name):
        if is_stdlib_module(module_name.split('.')[0]):
            return None
        mod_file = find_module_file(module_name, self.input_dir)
        if mod_file == None:
            init_file = find_package_init(module_name, self.input_dir)
            if init_file != None:
                mod_file = init_file
        if mod_file == None:
            return None
        f_in = open(mod_file, "r")
        text = f_in.read()
        f_in.close()
        lexer = Lexer(text)
        lexer.tokenize()
        parser = Parser(lexer.tokens)
        return parser.parse_module()

    def _gen_import(self, node, is_in_func):
        module_name = node.value
        alias_node = node.children[0]
        local_name = module_name.split('.')[0]
        if alias_node.type == "NAME":
            local_name = alias_node.value
        if is_stdlib_module(module_name.split('.')[0]):
            self.write_code("/* import " + module_name + " (stdlib) */;", is_in_func)
            return
        if module_name in self.processed_modules:
            self.write_code("t_" + local_name + " = t_init_module_" + module_name.replace('.', '_') + "();", is_in_func)
            return
        module_ast = self._load_module(module_name)
        if module_ast == None:
            self.write_code("/* import " + module_name + " (not found) */;", is_in_func)
            return
        self.processed_modules.append(module_name)
        prefix = module_name.replace('.', '_') + '_'
        init_func_name = "t_init_module_" + module_name.replace('.', '_')
        mod_var_name = "t_module_" + module_name.replace('.', '_')
        mod_globals = []
        collect_globals(module_ast, mod_globals)
        g_idx = 0
        while g_idx < len(mod_globals):
            g_name = mod_globals[g_idx]
            is_builtin = False
            b_idx = 0
            while b_idx < len(self.builtins_list):
                if self.builtins_list[b_idx] == g_name:
                    is_builtin = True
                    break
                b_idx = b_idx + 1
            if not is_builtin:
                self.write_header("TurboObject* t_" + prefix + g_name + " = NULL;\n")
            g_idx = g_idx + 1
        self.gen_module_init(module_name, module_ast)
        self.write_code("t_" + local_name + " = " + init_func_name + "();", is_in_func)

    def _gen_from_import(self, node, is_in_func):
        module_name = node.value
        imported = node.children
        if is_stdlib_module(module_name.split('.')[0]):
            self.write_code("/* from " + module_name + " import ... (stdlib) */;", is_in_func)
            return
        if not (module_name in self.processed_modules):
            module_ast = self._load_module(module_name)
            if module_ast == None:
                self.write_code("/* from " + module_name + " import ... (not found) */;", is_in_func)
                return
            self.processed_modules.append(module_name)
            prefix = module_name.replace('.', '_') + '_'
            mod_globals = []
            collect_globals(module_ast, mod_globals)
            g_idx = 0
            while g_idx < len(mod_globals):
                g_name = mod_globals[g_idx]
                is_builtin = False
                b_idx = 0
                while b_idx < len(self.builtins_list):
                    if self.builtins_list[b_idx] == g_name:
                        is_builtin = True
                        break
                    b_idx = b_idx + 1
                if not is_builtin:
                    self.write_header("TurboObject* t_" + prefix + g_name + " = NULL;\n")
                g_idx = g_idx + 1
            self.gen_module_init(module_name, module_ast)
        init_func_name = "t_init_module_" + module_name.replace('.', '_')
        i_idx = 0
        while i_idx < len(imported):
            item = imported[i_idx]
            if item.type == "NAME":
                orig_name = item.value
                self.write_code("t_" + orig_name + " = turbo_module_get(" + init_func_name + "(), \"" + orig_name + "\");", is_in_func)
            elif item.type == "IMPORT_AS":
                orig_name = item.children[0].value
                alias_name = item.children[1].value
                self.write_code("t_" + alias_name + " = turbo_module_get(" + init_func_name + "(), \"" + orig_name + "\");", is_in_func)
            i_idx = i_idx + 1

    def gen_module_init(self, module_name, module_ast):
        prefix = module_name.replace('.', '_') + '_'
        init_func_name = "t_init_module_" + module_name.replace('.', '_')
        stmts = module_ast.children
        s_idx = 0
        while s_idx < len(stmts):
            stmt = stmts[s_idx]
            if stmt.type == "DEF" or stmt.type == "ASYNC_DEF":
                old_is_gen = self.is_generator
                self.gen_func_def(stmt, "", prefix)
                self.is_generator = old_is_gen
            elif stmt.type == "CLASS":
                old_is_gen = self.is_generator
                self.gen_class_def(stmt, prefix)
                self.is_generator = old_is_gen
            s_idx = s_idx + 1
        self.write_header("TurboObject* " + init_func_name + "(void);\n")
        self.funcs = self.funcs + "TurboObject* " + init_func_name + "(void) {\n"
        self.funcs = self.funcs + "    static int _init_done = 0;\n"
        self.funcs = self.funcs + "    static TurboObject* _mod_obj = NULL;\n"
        self.funcs = self.funcs + "    if (_init_done) return _mod_obj;\n"
        self.funcs = self.funcs + "    _init_done = 1;\n"
        self.funcs = self.funcs + "    TurboObject* mod = make_module();\n"
        s_idx = 0
        while s_idx < len(stmts):
            stmt = stmts[s_idx]
            if stmt.type == "DEF" or stmt.type == "ASYNC_DEF":
                func_name = stmt.value
                self.funcs = self.funcs + "    " + "turbo_module_set(mod, \"" + func_name + "\", make_func(t_impl_" + prefix + func_name + ", \"" + func_name + "\"));\n"
            elif stmt.type == "CLASS":
                class_name = stmt.value
                self.funcs = self.funcs + "    " + "t_" + prefix + class_name + " = make_class(\"" + class_name + "\");\n"
                suite = stmt.children[0]
                m_idx = 0
                while m_idx < len(suite.children):
                    m_node = suite.children[m_idx]
                    if m_node.type == "DEF" or m_node.type == "ASYNC_DEF":
                        method_name = m_node.value
                        self.funcs = self.funcs + "    " + "turbo_class_add_method(t_" + prefix + class_name + ", \"" + method_name + "\", t_impl_" + prefix + class_name + "_" + method_name + ");\n"
                    m_idx = m_idx + 1
                self.funcs = self.funcs + "    " + "turbo_module_set(mod, \"" + class_name + "\", t_" + prefix + class_name + ");\n"
            elif stmt.type == "ASSIGN":
                target = stmt.children[0]
                val_node = stmt.children[1]
                if target.type == "NAME":
                    var_name = target.value
                    val_c = self.gen_expr(val_node)
                    self.funcs = self.funcs + "    " + "t_" + prefix + var_name + " = " + val_c + ";\n"
                    self.funcs = self.funcs + "    " + "turbo_module_set(mod, \"" + var_name + "\", t_" + prefix + var_name + ");\n"
            elif stmt.type == "AUGASSIGN":
                target = stmt.children[0]
                val_node = stmt.children[1]
                op = stmt.value
                if target.type == "NAME":
                    var_name = target.value
                    if op == '+=':
                        self.funcs = self.funcs + "    t_" + prefix + var_name + " = turbo_add(t_" + prefix + var_name + ", " + self.gen_expr(val_node) + ");\n"
                        self.funcs = self.funcs + "    turbo_module_set(mod, \"" + var_name + "\", t_" + prefix + var_name + ");\n"
            elif stmt.type == "EXPR":
                expr_c = self.gen_expr(stmt.children[0])
                self.funcs = self.funcs + "    " + "(void)(" + expr_c + ");\n"
            elif stmt.type == "IMPORT" or stmt.type == "IMPORT_LIST" or stmt.type == "FROM_IMPORT":
                pass
            else:
                pass
            s_idx = s_idx + 1
        self.funcs = self.funcs + "    _mod_obj = mod;\n"
        self.funcs = self.funcs + "    return mod;\n"
        self.funcs = self.funcs + "}\n\n"

    def gen_block_stmts(self, block_node, is_in_func):
        stmts = block_node.children
        s_idx = 0
        while s_idx < len(stmts):
            self.gen_stmt(stmts[s_idx], is_in_func)
            s_idx = s_idx + 1

    def gen_stmt(self, node, is_in_func):
        if node.type == "PASS":
            self.write_code("/* pass */;", is_in_func)
        elif node.type == "IMPORT":
            self._gen_import(node, is_in_func)
        elif node.type == "IMPORT_LIST":
            c_idx = 0
            while c_idx < len(node.children):
                self._gen_import(node.children[c_idx], is_in_func)
                c_idx = c_idx + 1
        elif node.type == "FROM_IMPORT":
            self._gen_from_import(node, is_in_func)
        elif node.type == "BREAK":
            if len(self.else_loop_stack) > 0:
                self.write_code(self.else_loop_stack[-1] + " = 1; break;", is_in_func)
            else:
                self.write_code("break;", is_in_func)
        elif node.type == "CONTINUE":
            self.write_code("continue;", is_in_func)
        elif node.type == "EXPR":
            expr_c = self.gen_expr(node.children[0])
            self.write_code("(void)(" + expr_c + ");", is_in_func)
        elif node.type == "ASSIGN":
            target = node.children[0]
            val_node = node.children[1]
            # Ignore sys_argv = sys.argv for bootstrap compatibility
            is_sys_argv_setup = False
            if target.type == "NAME" and target.value == "sys_argv":
                if val_node.type == "ATTR" and val_node.value == "argv":
                    if val_node.children[0].type == "NAME" and val_node.children[0].value == "sys":
                        is_sys_argv_setup = True
            
            if is_sys_argv_setup:
                self.write_code("/* ignored sys_argv = sys.argv */;", is_in_func)
            else:
                val_c = self.gen_expr(val_node)
                if target.type == "NAME":
                    self.write_code("t_" + target.value + " = " + val_c + ";", is_in_func)
                elif target.type == "ATTR":
                    obj_c = self.gen_expr(target.children[0])
                    self.write_code('turbo_setattr(' + obj_c + ', "' + target.value + '", ' + val_c + ');', is_in_func)
                elif target.type == "SUBSCRIPT":
                    obj_c = self.gen_expr(target.children[0])
                    idx_c = self.gen_expr(target.children[1])
                    self.write_code('turbo_setitem(' + obj_c + ', ' + idx_c + ', ' + val_c + ');', is_in_func)
        elif node.type == "AUGASSIGN":
            target = node.children[0]
            val_c = self.gen_expr(node.children[1])
            op = node.value
            if target.type == "NAME":
                if op == '+=':
                    self.write_code("t_" + target.value + " = turbo_add(t_" + target.value + ", " + val_c + ");", is_in_func)
                elif op == '-=':
                    self.write_code("t_" + target.value + " = turbo_sub(t_" + target.value + ", " + val_c + ");", is_in_func)
                elif op == '*=':
                    self.write_code("t_" + target.value + " = turbo_mul(t_" + target.value + ", " + val_c + ");", is_in_func)
                elif op == '/=':
                    self.write_code("t_" + target.value + " = turbo_div(t_" + target.value + ", " + val_c + ");", is_in_func)
                elif op == '%=':
                    self.write_code("t_" + target.value + " = turbo_mod(t_" + target.value + ", " + val_c + ");", is_in_func)
                elif op == '**=':
                    self.write_code("t_" + target.value + " = turbo_pow(t_" + target.value + ", " + val_c + ");", is_in_func)
                elif op == '//=':
                    self.write_code("t_" + target.value + " = turbo_floordiv(t_" + target.value + ", " + val_c + ");", is_in_func)
                elif op == '&=':
                    self.write_code("t_" + target.value + " = turbo_bitand(t_" + target.value + ", " + val_c + ");", is_in_func)
                elif op == '|=':
                    self.write_code("t_" + target.value + " = turbo_bitor(t_" + target.value + ", " + val_c + ");", is_in_func)
                elif op == '^=':
                    self.write_code("t_" + target.value + " = turbo_bitxor(t_" + target.value + ", " + val_c + ");", is_in_func)
                elif op == '>>=':
                    self.write_code("t_" + target.value + " = turbo_rshift(t_" + target.value + ", " + val_c + ");", is_in_func)
                elif op == '<<=':
                    self.write_code("t_" + target.value + " = turbo_lshift(t_" + target.value + ", " + val_c + ");", is_in_func)
                elif op == '@=':
                    self.write_code("t_" + target.value + " = turbo_matmul(t_" + target.value + ", " + val_c + ");", is_in_func)
        elif node.type == "RETURN":
            if self.is_generator and node.children[0].type != "CONST_NONE":
                self.gen_expr(node.children[0])
            if self.is_generator:
                self.write_code("return __yield_values;", is_in_func)
            else:
                val_c = self.gen_expr(node.children[0])
                self.write_code("return " + val_c + ";", is_in_func)
        elif node.type == "WHILE":
            cond_c = self.gen_expr(node.children[0])
            if len(node.children) > 2:
                flag_name = "_turbo_brk_" + str(self.else_loop_counter)
                self.else_loop_counter = self.else_loop_counter + 1
                self.else_loop_stack.append(flag_name)
                self.write_code("{", is_in_func)
                self.indent_level = self.indent_level + 1
                self.write_code("int " + flag_name + " = 0;", is_in_func)
                self.write_code("while (turbo_is_truthy(" + cond_c + ")) {", is_in_func)
                self.indent_level = self.indent_level + 1
                self.gen_block_stmts(node.children[1], is_in_func)
                self.indent_level = self.indent_level - 1
                self.write_code("}", is_in_func)
                self.write_code("if (!" + flag_name + ") {", is_in_func)
                self.indent_level = self.indent_level + 1
                self.gen_block_stmts(node.children[2], is_in_func)
                self.indent_level = self.indent_level - 1
                self.write_code("}", is_in_func)
                self.indent_level = self.indent_level - 1
                self.write_code("}", is_in_func)
                self.else_loop_stack.pop()
            else:
                self.write_code("while (turbo_is_truthy(" + cond_c + ")) {", is_in_func)
                self.indent_level = self.indent_level + 1
                self.gen_block_stmts(node.children[1], is_in_func)
                self.indent_level = self.indent_level - 1
                self.write_code("}", is_in_func)
        elif node.type == "FOR":
            var_name = node.value
            iter_c = self.gen_expr(node.children[0])
            do_else = len(node.children) > 2
            if do_else:
                flag_name = "_turbo_brk_" + str(self.else_loop_counter)
                self.else_loop_counter = self.else_loop_counter + 1
                self.else_loop_stack.append(flag_name)
            self.write_code("{", is_in_func)
            self.indent_level = self.indent_level + 1
            if do_else:
                self.write_code("int " + flag_name + " = 0;", is_in_func)
            self.write_code("TurboObject* _iter = " + iter_c + ";", is_in_func)
            self.write_code("if (_iter->type == TYPE_LIST) {", is_in_func)
            self.indent_level = self.indent_level + 1
            self.write_code("for (int _i = 0; _i < _iter->list_val.length; _i++) {", is_in_func)
            self.indent_level = self.indent_level + 1
            self.write_code("t_" + var_name + " = _iter->list_val.items[_i];", is_in_func)
            self.gen_block_stmts(node.children[1], is_in_func)
            self.indent_level = self.indent_level - 1
            self.write_code("}", is_in_func)
            self.indent_level = self.indent_level - 1
            self.write_code("} else if (_iter->type == TYPE_TUPLE) {", is_in_func)
            self.indent_level = self.indent_level + 1
            self.write_code("for (int _i = 0; _i < _iter->tuple_val.length; _i++) {", is_in_func)
            self.indent_level = self.indent_level + 1
            self.write_code("t_" + var_name + " = _iter->tuple_val.items[_i];", is_in_func)
            self.gen_block_stmts(node.children[1], is_in_func)
            self.indent_level = self.indent_level - 1
            self.write_code("}", is_in_func)
            self.indent_level = self.indent_level - 1
            self.write_code("} else if (_iter->type == TYPE_STR) {", is_in_func)
            self.indent_level = self.indent_level + 1
            self.write_code("for (int _i = 0; _i < _iter->str_val.length; _i++) {", is_in_func)
            self.indent_level = self.indent_level + 1
            self.write_code("char _tmp[2] = {_iter->str_val.chars[_i], '\\0'};", is_in_func)
            self.write_code("t_" + var_name + " = make_str(_tmp);", is_in_func)
            self.gen_block_stmts(node.children[1], is_in_func)
            self.indent_level = self.indent_level - 1
            self.write_code("}", is_in_func)
            self.indent_level = self.indent_level - 1
            self.write_code("}", is_in_func)
            if do_else:
                self.write_code("if (!" + flag_name + ") {", is_in_func)
                self.indent_level = self.indent_level + 1
                self.gen_block_stmts(node.children[2], is_in_func)
                self.indent_level = self.indent_level - 1
                self.write_code("}", is_in_func)
            self.indent_level = self.indent_level - 1
            self.write_code("}", is_in_func)
            if do_else:
                self.else_loop_stack.pop()
        elif node.type == "ASYNC_FOR":
            self.gen_stmt(ASTNode("FOR", node.value, node.children), is_in_func)
        elif node.type == "IF":
            cond_c = self.gen_expr(node.children[0])
            self.write_code("if (turbo_is_truthy(" + cond_c + ")) {", is_in_func)
            self.indent_level = self.indent_level + 1
            self.gen_block_stmts(node.children[1], is_in_func)
            self.indent_level = self.indent_level - 1
            if len(node.children) >= 3:
                else_node = node.children[2]
                if else_node.type == "IF":
                    self.write_code("} else {", is_in_func)
                    self.indent_level = self.indent_level + 1
                    self.gen_stmt(else_node, is_in_func)
                    self.indent_level = self.indent_level - 1
                else:
                    self.write_code("} else {", is_in_func)
                    self.indent_level = self.indent_level + 1
                    self.gen_block_stmts(else_node, is_in_func)
                    self.indent_level = self.indent_level - 1
            self.write_code("}", is_in_func)
        elif node.type == "RAISE":
            val_c = self.gen_expr(node.children[0])
            self.write_code("turbo_raise(" + val_c + ");", is_in_func)
        elif node.type == "ASSERT":
            test_c = self.gen_expr(node.children[0])
            if node.children[1].type == "CONST_NONE":
                self.write_code("if (!turbo_is_truthy(" + test_c + ")) { fprintf(stderr, \"AssertionError\\n\"); exit(1); }", is_in_func)
            else:
                msg_c = self.gen_expr(node.children[1])
                self.write_code("if (!turbo_is_truthy(" + test_c + ")) { TurboObject* _amsg = turbo_str(" + msg_c + "); fprintf(stderr, \"AssertionError: %s\\n\", _amsg->str_val.chars); exit(1); }", is_in_func)
        elif node.type == "DEL":
            target = node.children[0]
            if target.type == "SUBSCRIPT":
                obj_c = self.gen_expr(target.children[0])
                idx_c = self.gen_expr(target.children[1])
                self.write_code("turbo_delitem(" + obj_c + ", " + idx_c + ");", is_in_func)
            elif target.type == "NAME":
                self.write_code("t_" + target.value + " = turbo_none;", is_in_func)
            else:
                self.write_code("/* del */", is_in_func)
        elif node.type == "GLOBAL":
            if is_in_func:
                self.write_code("/* global: names resolved to module scope */", is_in_func)
        elif node.type == "NONLOCAL":
            if is_in_func:
                self.write_code("/* nonlocal: names resolved to enclosing function scope */", is_in_func)
        elif node.type == "WITH":
            expr_c = self.gen_expr(node.children[0])
            alias_node = node.children[1]
            body = node.children[2]
            self.write_code("{", is_in_func)
            self.indent_level = self.indent_level + 1
            self.write_code("TurboObject* _cm = " + expr_c + ";", is_in_func)
            if alias_node.type == "NAME":
                self.write_code("t_" + alias_node.value + " = turbo_call_method(_cm, \"__enter__\", 0, NULL);", is_in_func)
            else:
                self.write_code("turbo_call_method(_cm, \"__enter__\", 0, NULL);", is_in_func)
            self.write_code("int _exc_had = 0;", is_in_func)
            self.write_code("TurboObject* _exc_val = turbo_none;", is_in_func)
            self.write_code("jmp_buf* _saved_jmp = turbo_exception_jmp;", is_in_func)
            self.write_code("jmp_buf _try_jmp;", is_in_func)
            self.write_code("turbo_exception_jmp = &_try_jmp;", is_in_func)
            self.write_code("turbo_exception_value = turbo_none;", is_in_func)
            self.write_code("if (setjmp(_try_jmp) == 0) {", is_in_func)
            self.indent_level = self.indent_level + 1
            self.gen_block_stmts(body, is_in_func)
            self.indent_level = self.indent_level - 1
            self.write_code("} else {", is_in_func)
            self.indent_level = self.indent_level + 1
            self.write_code("_exc_had = 1;", is_in_func)
            self.write_code("_exc_val = turbo_exception_value;", is_in_func)
            self.indent_level = self.indent_level - 1
            self.write_code("}", is_in_func)
            self.write_code("turbo_exception_jmp = _saved_jmp;", is_in_func)
            self.write_code("if (_exc_had) {", is_in_func)
            self.indent_level = self.indent_level + 1
            self.write_code("TurboObject* _exc_type = _exc_val;", is_in_func)
            self.write_code("if (_exc_val->type == TYPE_INSTANCE) _exc_type = _exc_val->inst_val.class_obj;", is_in_func)
            self.write_code("if (!turbo_is_truthy(turbo_call_method(_cm, \"__exit__\", 3, (TurboObject*[]){_exc_type, _exc_val, turbo_none}))) {", is_in_func)
            self.indent_level = self.indent_level + 1
            self.write_code("turbo_raise(_exc_val);", is_in_func)
            self.indent_level = self.indent_level - 1
            self.write_code("}", is_in_func)
            self.indent_level = self.indent_level - 1
            self.write_code("} else {", is_in_func)
            self.indent_level = self.indent_level + 1
            self.write_code("turbo_call_method(_cm, \"__exit__\", 3, (TurboObject*[]){turbo_none, turbo_none, turbo_none});", is_in_func)
            self.indent_level = self.indent_level - 1
            self.write_code("}", is_in_func)
            self.indent_level = self.indent_level - 1
            self.write_code("}", is_in_func)
        elif node.type == "ASYNC_WITH":
            self.gen_stmt(ASTNode("WITH", "", node.children), is_in_func)
        elif node.type == "TRY":
            body = node.children[0]
            handlers_node = node.children[1]
            else_body = node.children[2]
            finally_body = node.children[3]
            self.write_code("{", is_in_func)
            self.indent_level = self.indent_level + 1
            self.write_code("int _exc_had = 0;", is_in_func)
            self.write_code("int _exc_handled = 0;", is_in_func)
            self.write_code("TurboObject* _exc_val = turbo_none;", is_in_func)
            self.write_code("jmp_buf* _saved_jmp = turbo_exception_jmp;", is_in_func)
            self.write_code("jmp_buf _try_jmp;", is_in_func)
            self.write_code("turbo_exception_jmp = &_try_jmp;", is_in_func)
            self.write_code("turbo_exception_value = turbo_none;", is_in_func)
            self.write_code("if (setjmp(_try_jmp) == 0) {", is_in_func)
            self.indent_level = self.indent_level + 1
            self.gen_block_stmts(body, is_in_func)
            self.indent_level = self.indent_level - 1
            self.write_code("} else {", is_in_func)
            self.indent_level = self.indent_level + 1
            self.write_code("_exc_had = 1;", is_in_func)
            self.write_code("_exc_val = turbo_exception_value;", is_in_func)
            self.indent_level = self.indent_level - 1
            self.write_code("}", is_in_func)
            self.write_code("turbo_exception_jmp = _saved_jmp;", is_in_func)
            if len(handlers_node.children) > 0:
                self.write_code("if (_exc_had) {", is_in_func)
                self.indent_level = self.indent_level + 1
                h_idx = 0
                while h_idx < len(handlers_node.children):
                    handler = handlers_node.children[h_idx]
                    exc_type = handler.children[0]
                    exc_var = handler.value
                    handler_body = handler.children[1]
                    if exc_type.type == "CONST_NONE":
                        self.write_code("if (!_exc_handled) {", is_in_func)
                    else:
                        exc_type_c = self.gen_expr(exc_type)
                        self.write_code("if (!_exc_handled && turbo_exception_matches(_exc_val, " + exc_type_c + ")) {", is_in_func)
                    self.indent_level = self.indent_level + 1
                    self.write_code("_exc_handled = 1;", is_in_func)
                    if exc_var != "":
                        self.write_code("t_" + exc_var + " = _exc_val;", is_in_func)
                    self.gen_block_stmts(handler_body, is_in_func)
                    self.indent_level = self.indent_level - 1
                    self.write_code("}", is_in_func)
                    h_idx = h_idx + 1
                self.write_code("if (!_exc_handled) {", is_in_func)
                self.indent_level = self.indent_level + 1
                self.write_code("turbo_raise(_exc_val);", is_in_func)
                self.indent_level = self.indent_level - 1
                self.write_code("}", is_in_func)
                self.indent_level = self.indent_level - 1
                self.write_code("}", is_in_func)
            if else_body.type != "CONST_NONE":
                self.write_code("if (!_exc_had) {", is_in_func)
                self.indent_level = self.indent_level + 1
                self.gen_block_stmts(else_body, is_in_func)
                self.indent_level = self.indent_level - 1
                self.write_code("}", is_in_func)
            if finally_body.type != "CONST_NONE":
                self.gen_block_stmts(finally_body, is_in_func)
            if len(handlers_node.children) == 0:
                self.write_code("if (_exc_had) { turbo_raise(_exc_val); }", is_in_func)
            self.indent_level = self.indent_level - 1
            self.write_code("}", is_in_func)
        elif node.type == "MATCH":
            subject_expr = self.gen_expr(node.children[0])
            self.write_code("{", is_in_func)
            self.indent_level = self.indent_level + 1
            self.write_code("TurboObject *__match_subject = " + subject_expr + ";", is_in_func)
            n = len(node.children)
            i = 1
            while i < n:
                case_node = node.children[i]
                pattern_node = case_node.children[0]
                body_node = case_node.children[1]
                is_first = (i == 1)
                if pattern_node.type == "PATTERN_LIT":
                    lit_expr = self.gen_expr(pattern_node.children[0])
                    cond = "turbo_is_truthy(turbo_eq(__match_subject, " + lit_expr + "))"
                    if is_first:
                        self.write_code("if (" + cond + ") {", is_in_func)
                    else:
                        self.write_code("} else if (" + cond + ") {", is_in_func)
                elif pattern_node.type == "PATTERN_CAP":
                    if is_first:
                        self.write_code("if (1) {", is_in_func)
                    else:
                        self.write_code("} else {", is_in_func)
                elif pattern_node.type == "PATTERN_WC":
                    if is_first:
                        self.write_code("if (1) {", is_in_func)
                    else:
                        self.write_code("} else {", is_in_func)
                self.indent_level = self.indent_level + 1
                if pattern_node.type == "PATTERN_CAP":
                    self.write_code("t_" + pattern_node.value + " = __match_subject;", is_in_func)
                self.gen_block_stmts(body_node, is_in_func)
                self.indent_level = self.indent_level - 1
                i = i + 1
            self.write_code("}", is_in_func)
            self.indent_level = self.indent_level - 1
            self.write_code("}", is_in_func)
        elif node.type == "DEF" or node.type == "ASYNC_DEF":
            self.gen_func_def(node, "", "")
        elif node.type == "CLASS":
            self.gen_class_def(node, "")

def main():
    if len(sys_argv) < 4:
        print("Usage: turbo <input_file> -o <output_file>")
        return
    
    input_file = sys_argv[1]
    output_file = ""
    
    i = 2
    while i < len(sys_argv):
        if sys_argv[i] == "-o":
            if i + 1 < len(sys_argv):
                output_file = sys_argv[i+1]
                break
        i = i + 1
        
    if output_file == "":
        print("Error: output file not specified with -o")
        return
        
    f_in = open(input_file, "r")
    text = f_in.read()
    f_in.close()
    
    lexer = Lexer(text)
    lexer.tokenize()
    
    parser = Parser(lexer.tokens)
    module_ast = parser.parse_module()
    
    input_dir = ""
    last_slash = -1
    s_idx = 0
    while s_idx < len(input_file):
        if input_file[s_idx] == '/':
            last_slash = s_idx
        s_idx = s_idx + 1
    if last_slash >= 0:
        input_dir = input_file[0:last_slash]
    if input_dir == "":
        input_dir = "."
    
    cg = CodeGen()
    cg.input_dir = input_dir
    
    globals_list = []
    collect_globals(module_ast, globals_list)
    
    # Declare globals in C header
    g_idx = 0
    builtins = cg.builtins_list
    while g_idx < len(globals_list):
        g_name = globals_list[g_idx]
        is_builtin = False
        b_idx = 0
        while b_idx < len(builtins):
            if builtins[b_idx] == g_name:
                is_builtin = True
                break
            b_idx = b_idx + 1
            
        if not is_builtin:
            cg.write_header("TurboObject* t_" + g_name + " = NULL;\n")
        g_idx = g_idx + 1
        
    # Generate C main body
    cg.indent_level = 1
    cg.gen_block_stmts(module_ast, False)
    
    # Combine outputs
    c_source = '#include "runtime.h"\n#include <stdlib.h>\n\n'
    c_source = c_source + cg.header + "\n"
    c_source = c_source + cg.funcs + "\n"
    c_source = c_source + "void turbo_main(void) {\n"
    c_source = c_source + cg.main_body
    c_source = c_source + "}\n\n"
    c_source = c_source + "int main(int argc, char** argv) {\n"
    c_source = c_source + "    turbo_init();\n"
    c_source = c_source + "    t_sys_argv = make_list();\n"
    c_source = c_source + "    for (int i = 0; i < argc; i++) {\n"
    c_source = c_source + "        turbo_list_append(t_sys_argv, make_str(argv[i]));\n"
    c_source = c_source + "    }\n"
    c_source = c_source + "    turbo_main();\n"
    c_source = c_source + "    return 0;\n"
    c_source = c_source + "}\n"
    
    f_out = open(output_file, "w")
    f_out.write(c_source)
    f_out.close()

main()
