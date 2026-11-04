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

    def _tokenize_expr(self, text, line_num):
        tokens = []
        p = 0
        while p < len(text):
            c = text[p]
            if c == ' ' or c == '\t':
                p = p + 1
                continue
            if p + 1 < len(text):
                two = text[p:p+2]
                if two in ('==', '!=', '<=', '>=', '+=', '-=', '*=', '/=', '%=', '**', '//', '<<', '>>', '&=', '|=', '^=', ':=', '->', '@='):
                    tokens.append(Token(two, two, line_num))
                    p = p + 2
                    continue
            if c in '+-*/%=<>()[]{}:.,&|^~!@':
                tokens.append(Token(c, c, line_num))
                p = p + 1
                continue
            if c >= '0' and c <= '9':
                val = ""
                is_float = False
                while p < len(text):
                    ch = text[p]
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
                    elif ch == 'j' or ch == 'J':
                        val = val + ch
                        p = p + 1
                        tokens.append(Token("IMAG", val, line_num))
                        break
                    else:
                        break
                if is_float:
                    tokens.append(Token("FLOAT", val, line_num))
                else:
                    if val != "":
                        tokens.append(Token("NUMBER", val, line_num))
                continue
            if c == '"' or c == "'":
                quote = c
                val = ""
                p = p + 1
                while p < len(text) and text[p] != quote:
                    if text[p] == '\\':
                        p = p + 1
                        if p < len(text):
                            ec = text[p]
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
                        val = val + text[p]
                        p = p + 1
                p = p + 1
                tokens.append(Token("STRING", val, line_num))
                continue
            if (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z') or c == '_':
                val = ""
                while p < len(text) and ((text[p] >= 'a' and text[p] <= 'z') or (text[p] >= 'A' and text[p] <= 'Z') or (text[p] >= '0' and text[p] <= '9') or text[p] == '_'):
                    val = val + text[p]
                    p = p + 1
                keywords = ["def", "class", "if", "elif", "else", "while", "for", "in", "return", "and", "or", "not", "pass", "None", "True", "False", "import", "break", "continue", "raise", "assert", "del", "global", "nonlocal", "try", "except", "finally", "with", "as", "from", "async", "await", "match", "case", "yield", "lambda"]
                if val in keywords:
                    tokens.append(Token(val, val, line_num))
                else:
                    tokens.append(Token("NAME", val, line_num))
                continue
            p = p + 1
        return tokens

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
                p = p + 1
                segments = []
                lit = ""
                while p < len(line) and line[p] != quote:
                    if line[p] == '\\':
                        p = p + 1
                        if p < len(line):
                            ec = line[p]
                            if ec == 'n':
                                lit = lit + "\n"
                            elif ec == 't':
                                lit = lit + "\t"
                            elif ec == 'r':
                                lit = lit + "\r"
                            else:
                                lit = lit + ec
                        p = p + 1
                    elif line[p] == '{':
                        p = p + 1
                        if p < len(line) and line[p] == '{':
                            lit = lit + '{'
                            p = p + 1
                        else:
                            segments.append(("lit", lit))
                            lit = ""
                            depth = 1
                            expr_text = ""
                            while p < len(line) and depth > 0:
                                if line[p] == '{':
                                    depth = depth + 1
                                    expr_text = expr_text + '{'
                                    p = p + 1
                                elif line[p] == '}':
                                    depth = depth - 1
                                    if depth > 0:
                                        expr_text = expr_text + '}'
                                    p = p + 1
                                elif line[p] == '"' or line[p] == "'":
                                    q = line[p]
                                    expr_text = expr_text + q
                                    p = p + 1
                                    while p < len(line) and line[p] != q:
                                        if line[p] == '\\':
                                            expr_text = expr_text + '\\'
                                            p = p + 1
                                            if p < len(line):
                                                expr_text = expr_text + line[p]
                                                p = p + 1
                                        else:
                                            expr_text = expr_text + line[p]
                                            p = p + 1
                                    if p < len(line):
                                        expr_text = expr_text + line[p]
                                        p = p + 1
                                else:
                                    expr_text = expr_text + line[p]
                                    p = p + 1
                            segments.append(("expr", expr_text))
                            lit = ""
                    elif line[p] == '}':
                        p = p + 1
                        if p < len(line) and line[p] == '}':
                            lit = lit + '}'
                            p = p + 1
                    else:
                        lit = lit + line[p]
                        p = p + 1
                p = p + 1
                segments.append(("lit", lit))
                seg_idx = 0
                while seg_idx < len(segments):
                    seg_type = segments[seg_idx][0]
                    seg_val = segments[seg_idx][1]
                    if seg_type == "lit" and seg_val != "":
                        if seg_idx > 0:
                            self.tokens.append(Token("+", "+", line_num))
                        self.tokens.append(Token("STRING", seg_val, line_num))
                    elif seg_type == "expr" and seg_val != "":
                        if seg_idx > 0:
                            self.tokens.append(Token("+", "+", line_num))
                        self.tokens.append(Token("NAME", "str", line_num))
                        self.tokens.append(Token("(", "(", line_num))
                        expr_tokens = self._tokenize_expr(seg_val, line_num)
                        et_idx = 0
                        while et_idx < len(expr_tokens):
                            self.tokens.append(expr_tokens[et_idx])
                            et_idx = et_idx + 1
                        self.tokens.append(Token(")", ")", line_num))
                    seg_idx = seg_idx + 1
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
            
            # Numeric literals (int, float, hex, octal, binary, complex imaginary)
            if c >= '0' and c <= '9':
                val = ""
                is_float = False
                has_j = False
                # Check for hex / octal / binary prefix
                if c == '0' and p + 1 < len(line):
                    peek = line[p + 1]
                    if peek == 'x' or peek == 'X':
                        hex_str = ''
                        p = p + 2
                        while p < len(line):
                            ch = line[p]
                            if (ch >= '0' and ch <= '9') or (ch >= 'a' and ch <= 'f') or (ch >= 'A' and ch <= 'F'):
                                hex_str = hex_str + ch
                                p = p + 1
                            else:
                                break
                        if hex_str == '':
                            val = '0'
                        else:
                            # Convert hex to decimal (avoid int() with base arg)
                            dec_val = 0
                            h_idx = 0
                            while h_idx < len(hex_str):
                                ch = hex_str[h_idx]
                                if ch >= '0' and ch <= '9':
                                    d = ord(ch) - 48
                                elif ch >= 'a' and ch <= 'f':
                                    d = ord(ch) - 87
                                else:
                                    d = ord(ch) - 55
                                dec_val = dec_val * 16 + d
                                h_idx = h_idx + 1
                            val = str(dec_val)
                        self.tokens.append(Token("NUMBER", val, line_num))
                        continue
                    elif peek == 'o' or peek == 'O':
                        octal_str = ''
                        p = p + 2
                        while p < len(line):
                            ch = line[p]
                            if ch >= '0' and ch <= '7':
                                octal_str = octal_str + ch
                                p = p + 1
                            else:
                                break
                        if octal_str == '':
                            val = '0'
                        else:
                            dec_val = 0
                            h_idx = 0
                            while h_idx < len(octal_str):
                                d = ord(octal_str[h_idx]) - 48
                                dec_val = dec_val * 8 + d
                                h_idx = h_idx + 1
                            val = str(dec_val)
                        self.tokens.append(Token("NUMBER", val, line_num))
                        continue
                    elif peek == 'b' or peek == 'B':
                        bin_str = ''
                        p = p + 2
                        while p < len(line):
                            ch = line[p]
                            if ch == '0' or ch == '1':
                                bin_str = bin_str + ch
                                p = p + 1
                            else:
                                break
                        if bin_str == '':
                            val = '0'
                        else:
                            dec_val = 0
                            h_idx = 0
                            while h_idx < len(bin_str):
                                d = ord(bin_str[h_idx]) - 48
                                dec_val = dec_val * 2 + d
                                h_idx = h_idx + 1
                            val = str(dec_val)
                        self.tokens.append(Token("NUMBER", val, line_num))
                        continue
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
            # Handle bare tuple: a, b = ... or a, b (expression)
            if self.peek().type == ',':
                elements = [expr]
                while self.try_match(','):
                    if self.peek().type in (')', ']', '}', ':', ';', 'NEWLINE', 'EOF'):
                        break
                    elements.append(self.parse_expr())
                if len(elements) > 1:
                    expr = ASTNode("TUPLE", "", elements)
            next_t = self.peek()
            if next_t.type == ':=':
                self.pos = self.pos + 1
                value = self.parse_expr()
                self.consume('NEWLINE')
                return ASTNode("ASSIGN", "", [expr, value])
            elif next_t.type == ':':
                # Annotation: x: type  or  x: type = value
                self.pos = self.pos + 1
                type_expr = self.parse_expr()
                default_node = ASTNode("CONST_NONE", "", [])
                if self.peek().type == '=':
                    self.pos = self.pos + 1
                    default_node = self.parse_expr()
                self.consume('NEWLINE')
                if expr.type == "NAME":
                    return ASTNode("ANN_ASSIGN", expr.value, [type_expr, default_node])
                return ASTNode("EXPR", "", [expr])
            elif next_t.type == '=':
                self.pos = self.pos + 1
                value = self.parse_expr()
                # Handle tuple on right side: ... = a, b
                if self.peek().type == ',':
                    elements = [value]
                    while self.try_match(','):
                        if self.peek().type in (')', ']', '}', ':', ';', 'NEWLINE', 'EOF'):
                            break
                        elements.append(self.parse_expr())
                    if len(elements) > 1:
                        value = ASTNode("TUPLE", "", elements)
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
    if node.type == "ANN_ASSIGN":
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
        return

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
        elif target.type == "TUPLE":
            for elem in target.children:
                if elem.type == "NAME":
                    name = elem.value
                    found = False
                    i = 0
                    while i < len(locals_list):
                        if locals_list[i] == name:
                            found = True
                            break
                        i = i + 1
                    if not found:
                        locals_list.append(name)
                elif elem.type == "TUPLE":
                    # Nested tuple unpacking: recurse into the helper
                    collect_locals(elem, locals_list)
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

def escape_llvm_string(s):
    res = ""
    i = 0
    hex_chars = "0123456789ABCDEF"
    while i < len(s):
        c = s[i]
        code = ord(c)
        if code >= 32 and code <= 126 and c != '"' and c != '\\':
            res = res + c
        else:
            b = code & 255
            hi = (b >> 4) & 15
            lo = b & 15
            res = res + "\\" + hex_chars[hi] + hex_chars[lo]
        i = i + 1
    return res

def format_llvm_float(s):
    has_dot = False
    has_exp = False
    i = 0
    while i < len(s):
        c = s[i]
        if c == '.':
            has_dot = True
        elif c == 'e' or c == 'E':
            has_exp = True
        i = i + 1
    if not has_dot and not has_exp:
        return s + ".0"
    if has_dot and not has_exp:
        if s[len(s) - 1] == '.':
            return s + "0"
        if s[0] == '.':
            return "0" + s
        return s
    if has_exp and not has_dot:
        res = ""
        i = 0
        inserted = False
        while i < len(s):
            c = s[i]
            if (c == 'e' or c == 'E') and not inserted:
                res = res + ".0e"
                inserted = True
            else:
                res = res + c
            i = i + 1
        return res
    res = s
    if res[0] == '.':
        res = "0" + res
    return res

class CodeGen:
    def __init__(self):
        self.str_defs = []
        self.str_map = {}
        self.str_counter = 0
        self.global_vars = []
        self.raw_globals = []
        self.funcs_list = []
        self.cur_lines = []
        self.main_lines = []
        self.local_vars = []
        self.local_allocas = []
        self.main_allocas = []
        self.sized_allocas = []
        self.main_sized_allocas = []
        self.typed_allocas = []
        self.main_typed_allocas = []
        self.loop_stack = []
        self.try_depth = 0
        self.is_generator = False
        self.lambda_counter = 0
        self.comp_counter = 0
        self.reg_counter = 0
        self.label_counter = 0
        self.block_terminated = False
        self.mod_prefix = ""
        self.mod_local_globals = []
        self.processed_modules = []
        self.direct_funcs = {}
        self.local_direct_funcs = {}
        self.raw_int_funcs = {}
        self.range_loop_vars = {}
        self.input_dir = "."
        self.builtins_list = ["print", "len", "str", "int", "ord", "chr", "range", "open", "sys_argv", "input", "type", "isinstance", "hasattr", "getattr", "setattr", "repr", "abs", "round", "pow", "hex", "bin", "oct", "float", "bool", "list", "dict", "tuple", "set", "super", "iter", "next", "all", "any", "sum", "min", "max", "sorted", "reversed", "enumerate", "zip", "map", "filter"]
        self.pending_native_vars = None
        self._var_types = {}
        self._native_temp_counter = 0

    def new_reg(self):
        self.reg_counter = self.reg_counter + 1
        return "%r" + str(self.reg_counter)

    def new_label(self, prefix):
        self.label_counter = self.label_counter + 1
        return prefix + "_" + str(self.label_counter)

    def add_global(self, g_name):
        i = 0
        while i < len(self.global_vars):
            if self.global_vars[i] == g_name:
                return
            i = i + 1
        self.global_vars.append(g_name)

    def is_builtin_name(self, name):
        i = 0
        while i < len(self.builtins_list):
            if self.builtins_list[i] == name:
                return True
            i = i + 1
        return False

    def is_mod_global(self, name):
        if self.mod_prefix == "":
            return False
        i = 0
        while i < len(self.mod_local_globals):
            if self.mod_local_globals[i] == name:
                return True
            i = i + 1
        return False

    def is_local(self, v_name, is_in_func):
        if not is_in_func:
            return False
        i = 0
        while i < len(self.local_vars):
            if self.local_vars[i] == v_name:
                return True
            i = i + 1
        return False

    def ensure_alloca(self, v_name, is_in_func):
        if is_in_func:
            i = 0
            while i < len(self.local_allocas):
                if self.local_allocas[i] == v_name:
                    return
                i = i + 1
            self.local_allocas.append(v_name)
        else:
            i = 0
            while i < len(self.main_allocas):
                if self.main_allocas[i] == v_name:
                    return
                i = i + 1
            self.main_allocas.append(v_name)

    def ensure_typed_alloca(self, a_name, llvm_ty, is_in_func):
        if is_in_func:
            i = 0
            while i < len(self.typed_allocas):
                if self.typed_allocas[i][0] == a_name:
                    return
                i = i + 1
            self.typed_allocas.append([a_name, llvm_ty])
        else:
            i = 0
            while i < len(self.main_typed_allocas):
                if self.main_typed_allocas[i][0] == a_name:
                    return
                i = i + 1
            self.main_typed_allocas.append([a_name, llvm_ty])

    def ensure_sized_alloca(self, a_name, count, is_in_func):
        if is_in_func:
            self.sized_allocas.append([a_name, count])
        else:
            self.main_sized_allocas.append([a_name, count])

    def get_var_ptr(self, v_name, is_in_func):
        if self.is_local(v_name, is_in_func):
            self.ensure_alloca(v_name, True)
            return "%loc_" + v_name
        if self.is_mod_global(v_name):
            full_g = self.mod_prefix + v_name
            self.add_global(full_g)
            return "@t_" + full_g
        if not self.is_builtin_name(v_name):
            self.add_global(v_name)
        return "@t_" + v_name

    def emit_load_var(self, v_name, is_in_func):
        ptr = self.get_var_ptr(v_name, is_in_func)
        r = self.new_reg()
        self.emit(r + " = load i64, i64* " + ptr, is_in_func)
        return r

    def emit_store_var(self, v_name, val_reg, is_in_func):
        ptr = self.get_var_ptr(v_name, is_in_func)
        self.emit("store i64 " + val_reg + ", i64* " + ptr, is_in_func)

    def emit(self, line, is_in_func):
        if self.block_terminated:
            return
        if is_in_func:
            self.cur_lines.append("  " + line)
        else:
            self.main_lines.append("  " + line)

    def emit_label(self, lbl, is_in_func):
        if not self.block_terminated:
            self.emit("br label %" + lbl, is_in_func)
        if is_in_func:
            self.cur_lines.append(lbl + ":")
        else:
            self.main_lines.append(lbl + ":")
        self.block_terminated = False

    def emit_br(self, lbl, is_in_func):
        if not self.block_terminated:
            self.emit("br label %" + lbl, is_in_func)
            self.block_terminated = True

    def emit_cond_br(self, cond_i1, true_lbl, false_lbl, is_in_func):
        if not self.block_terminated:
            self.emit("br i1 " + cond_i1 + ", label %" + true_lbl + ", label %" + false_lbl, is_in_func)
            self.block_terminated = True

    def emit_ret(self, val_i64, is_in_func):
        if not self.block_terminated:
            t_idx = 0
            while t_idx < self.try_depth:
                self.emit("call void @turbo_try_pop()", is_in_func)
                t_idx = t_idx + 1
            self.emit("ret i64 " + val_i64, is_in_func)
            self.block_terminated = True

    def get_str_ptr(self, s, is_in_func):
        if s in self.str_map:
            info = self.str_map[s]
            sym = info[0]
            slen = info[1]
        else:
            self.str_counter = self.str_counter + 1
            sym = "@.str." + str(self.str_counter)
            slen = len(s) + 1
            escaped = escape_llvm_string(s)
            defn = sym + ' = private unnamed_addr constant [' + str(slen) + ' x i8] c"' + escaped + '\\00", align 1'
            self.str_defs.append(defn)
            self.str_map[s] = [sym, slen]
        r = self.new_reg()
        self.emit(r + " = getelementptr inbounds [" + str(slen) + " x i8], [" + str(slen) + " x i8]* " + sym + ", i64 0, i64 0", is_in_func)
        return r

    def emit_truthy_i1(self, val_reg, is_in_func):
        r_call = self.new_reg()
        self.emit(r_call + " = call i32 @turbo_is_truthy(i64 " + val_reg + ")", is_in_func)
        r_cmp = self.new_reg()
        self.emit(r_cmp + " = icmp ne i32 " + r_call + ", 0", is_in_func)
        return r_cmp

    def emit_args_array(self, arg_regs, is_in_func):
        argc = len(arg_regs)
        if argc == 0:
            return "null"
        self.reg_counter = self.reg_counter + 1
        arr_name = "argarr_" + str(self.reg_counter)
        self.ensure_sized_alloca(arr_name, argc, is_in_func)
        arr_ptr = "%loc_" + arr_name
        i = 0
        while i < argc:
            elem_ptr = self.new_reg()
            self.emit(elem_ptr + " = getelementptr inbounds [" + str(argc) + " x i64], [" + str(argc) + " x i64]* " + arr_ptr + ", i64 0, i64 " + str(i), is_in_func)
            self.emit("store i64 " + arg_regs[i] + ", i64* " + elem_ptr, is_in_func)
            i = i + 1
        first_ptr = self.new_reg()
        self.emit(first_ptr + " = getelementptr inbounds [" + str(argc) + " x i64], [" + str(argc) + " x i64]* " + arr_ptr + ", i64 0, i64 0", is_in_func)
        return first_ptr

    def emit_box_int(self, i64_val, is_in_func):
        r_mask = self.new_reg()
        self.emit(r_mask + " = and i64 " + i64_val + ", 281474976710655", is_in_func)
        r_box = self.new_reg()
        self.emit(r_box + " = or i64 " + r_mask + ", -2251799813685248", is_in_func)
        return r_box

    def emit_unbox_int(self, tv_val, is_in_func):
        r_shl = self.new_reg()
        self.emit(r_shl + " = shl i64 " + tv_val + ", 16", is_in_func)
        r_ashr = self.new_reg()
        self.emit(r_ashr + " = ashr i64 " + r_shl + ", 16", is_in_func)
        return r_ashr

    def emit_box_float(self, dbl_val, is_in_func):
        r_bc = self.new_reg()
        self.emit(r_bc + " = bitcast double " + dbl_val + " to i64", is_in_func)
        return r_bc

    def _gen_range_bound(self, node, is_in_func):
        if node.type == "CONST_INT":
            return str(int(node.value))
        if node.type == "BINOP":
            op = node.value
            if op in ("+", "-", "*"):
                left = self._gen_range_bound(node.children[0], is_in_func)
                right = self._gen_range_bound(node.children[1], is_in_func)
                if left != None and right != None:
                    r = self.new_reg()
                    llvm_op = "add"
                    if op == "-":
                        llvm_op = "sub"
                    elif op == "*":
                        llvm_op = "mul"
                    self.emit(r + " = " + llvm_op + " i64 " + left + ", " + right, is_in_func)
                    return r
        v = self.gen_expr(node, is_in_func)
        return self.emit_unbox_int(v, is_in_func)

    def gen_expr(self, node, is_in_func):
        if node.type == "CONST_INT":
            try:
                ival = int(node.value)
                if ival >= -140737488355328 and ival <= 140737488355327:
                    if ival >= 0:
                        return str(-2251799813685248 + ival)
                    else:
                        return str(-2251799813685248 + (281474976710656 + ival))
            except:
                pass
            sp = self.get_str_ptr(node.value, is_in_func)
            r = self.new_reg()
            self.emit(r + " = call i64 @make_int(i8* " + sp + ")", is_in_func)
            return r
        elif node.type == "CONST_FLOAT":
            f_lit = format_llvm_float(node.value)
            r = self.new_reg()
            self.emit(r + " = bitcast double " + f_lit + " to i64", is_in_func)
            return r
        elif node.type == "CONST_IMAG":
            imag_val = node.value
            if imag_val[len(imag_val) - 1] == 'j' or imag_val[len(imag_val) - 1] == 'J':
                imag_val = imag_val[0:len(imag_val) - 1]
            if imag_val == "":
                imag_val = "1.0"
            else:
                imag_val = format_llvm_float(imag_val)
            r = self.new_reg()
            self.emit(r + " = call i64 @make_complex(double 0.0, double " + imag_val + ")", is_in_func)
            return r
        elif node.type == "CONST_BYTES":
            sp = self.get_str_ptr(node.value, is_in_func)
            r = self.new_reg()
            self.emit(r + " = call i64 @make_bytes_from_str(i8* " + sp + ")", is_in_func)
            return r
        elif node.type == "CONST_STR":
            if node.value == "":
                r = self.new_reg()
                self.emit(r + " = load i64, i64* @_cached_str_empty", is_in_func)
                return r
            sp = self.get_str_ptr(node.value, is_in_func)
            r = self.new_reg()
            self.emit(r + " = call i64 @make_str(i8* " + sp + ")", is_in_func)
            return r
        elif node.type == "CONST_NONE":
            return "-1688849860263936"
        elif node.type == "CONST_BOOL":
            if node.value == "True":
                return "-1688849860263933"
            else:
                return "-1688849860263934"
        elif node.type == "NAME":
            return self.emit_load_var(node.value, is_in_func)
        elif node.type == "BINOP":
            left = self.gen_expr(node.children[0], is_in_func)
            right = self.gen_expr(node.children[1], is_in_func)
            op = node.value
            fn = "@turbo_add"
            if op == '+':
                fn = "@turbo_add"
            elif op == '-':
                fn = "@turbo_sub"
            elif op == '*':
                fn = "@turbo_mul"
            elif op == '/':
                fn = "@turbo_div"
            elif op == '%':
                fn = "@turbo_mod"
            elif op == '**':
                fn = "@turbo_pow"
            elif op == '//':
                fn = "@turbo_floordiv"
            elif op == '&':
                fn = "@turbo_bitand"
            elif op == '|':
                fn = "@turbo_bitor"
            elif op == '^':
                fn = "@turbo_bitxor"
            elif op == '<<':
                fn = "@turbo_lshift"
            elif op == '>>':
                fn = "@turbo_rshift"
            elif op == '@':
                fn = "@turbo_matmul"
            r = self.new_reg()
            self.emit(r + " = call i64 " + fn + "(i64 " + left + ", i64 " + right + ")", is_in_func)
            return r
        elif node.type == "UNARY":
            operand = self.gen_expr(node.children[0], is_in_func)
            op = node.value
            if op == '-':
                r = self.new_reg()
                self.emit(r + " = call i64 @turbo_sub(i64 -2251799813685248, i64 " + operand + ")", is_in_func)
                return r
            elif op == '+':
                return operand
            elif op == '~':
                r = self.new_reg()
                self.emit(r + " = call i64 @turbo_bitnot(i64 " + operand + ")", is_in_func)
                return r
        elif node.type == "COMP":
            left = self.gen_expr(node.children[0], is_in_func)
            right = self.gen_expr(node.children[1], is_in_func)
            op = node.value
            if op == 'in':
                r_in = self.new_reg()
                self.emit(r_in + " = call i32 @turbo_in(i64 " + left + ", i64 " + right + ")", is_in_func)
                r = self.new_reg()
                self.emit(r + " = call i64 @make_bool(i32 " + r_in + ")", is_in_func)
                return r
            fn = "@turbo_eq"
            if op == '==':
                fn = "@turbo_eq"
            elif op == '!=':
                fn = "@turbo_ne"
            elif op == '<':
                fn = "@turbo_lt"
            elif op == '>':
                fn = "@turbo_gt"
            elif op == '<=':
                fn = "@turbo_lte"
            elif op == '>=':
                fn = "@turbo_gte"
            r = self.new_reg()
            self.emit(r + " = call i64 " + fn + "(i64 " + left + ", i64 " + right + ")", is_in_func)
            return r
        elif node.type == "NOT":
            operand = self.gen_expr(node.children[0], is_in_func)
            r = self.new_reg()
            self.emit(r + " = call i64 @turbo_not(i64 " + operand + ")", is_in_func)
            return r
        elif node.type == "AND":
            res_var = "_and_" + str(self.reg_counter + 1)
            self.reg_counter = self.reg_counter + 1
            self.ensure_alloca(res_var, is_in_func)
            res_ptr = "%loc_" + res_var
            lh = self.gen_expr(node.children[0], is_in_func)
            self.emit("store i64 " + lh + ", i64* " + res_ptr, is_in_func)
            c_i1 = self.emit_truthy_i1(lh, is_in_func)
            lbl_rhs = self.new_label("and_rhs")
            lbl_end = self.new_label("and_end")
            self.emit_cond_br(c_i1, lbl_rhs, lbl_end, is_in_func)
            self.emit_label(lbl_rhs, is_in_func)
            rh = self.gen_expr(node.children[1], is_in_func)
            self.emit("store i64 " + rh + ", i64* " + res_ptr, is_in_func)
            self.emit_br(lbl_end, is_in_func)
            self.emit_label(lbl_end, is_in_func)
            r = self.new_reg()
            self.emit(r + " = load i64, i64* " + res_ptr, is_in_func)
            return r
        elif node.type == "OR":
            res_var = "_or_" + str(self.reg_counter + 1)
            self.reg_counter = self.reg_counter + 1
            self.ensure_alloca(res_var, is_in_func)
            res_ptr = "%loc_" + res_var
            lh = self.gen_expr(node.children[0], is_in_func)
            self.emit("store i64 " + lh + ", i64* " + res_ptr, is_in_func)
            c_i1 = self.emit_truthy_i1(lh, is_in_func)
            lbl_rhs = self.new_label("or_rhs")
            lbl_end = self.new_label("or_end")
            self.emit_cond_br(c_i1, lbl_end, lbl_rhs, is_in_func)
            self.emit_label(lbl_rhs, is_in_func)
            rh = self.gen_expr(node.children[1], is_in_func)
            self.emit("store i64 " + rh + ", i64* " + res_ptr, is_in_func)
            self.emit_br(lbl_end, is_in_func)
            self.emit_label(lbl_end, is_in_func)
            r = self.new_reg()
            self.emit(r + " = load i64, i64* " + res_ptr, is_in_func)
            return r
        elif node.type == "CALL":
            func_node = node.children[0]
            args_list = node.children[1].children
            argc = len(args_list)
            if func_node.type == "NAME" and func_node.value == "sum" and argc == 1:
                arg0 = args_list[0]
                if arg0.type == "LISTCOMP" or arg0.type == "GENCOMP":
                    return self._emit_fused_sum_comp(arg0, is_in_func)
            arg_regs = []
            a_idx = 0
            while a_idx < argc:
                arg_regs.append(self.gen_expr(args_list[a_idx], is_in_func))
                a_idx = a_idx + 1
            if func_node.type == "ATTR":
                obj_reg = self.gen_expr(func_node.children[0], is_in_func)
                method_name = func_node.value
                m_ptr = self.get_str_ptr(method_name, is_in_func)
                args_ptr = self.emit_args_array(arg_regs, is_in_func)
                r = self.new_reg()
                self.emit(r + " = call i64 @turbo_call_method(i64 " + obj_reg + ", i8* " + m_ptr + ", i32 " + str(argc) + ", i64* " + args_ptr + ")", is_in_func)
                return r
            elif func_node.type == "NAME":
                fn_name = func_node.value
                lookup_key = ""
                if fn_name in self.local_direct_funcs:
                    lookup_key = self.local_direct_funcs[fn_name]
                elif (self.mod_prefix + fn_name) in self.direct_funcs and not self.is_local(fn_name, is_in_func):
                    lookup_key = self.mod_prefix + fn_name
                if lookup_key != "" and lookup_key in self.direct_funcs:
                    df_info = self.direct_funcs[lookup_key]
                    ssa_sym = df_info[0]
                    expected_argc = df_info[1]
                    call_args = []
                    ca_i = 0
                    while ca_i < expected_argc:
                        if ca_i < argc:
                            call_args.append("i64 " + arg_regs[ca_i])
                        else:
                            call_args.append("i64 -1688849860263936")
                        ca_i = ca_i + 1
                    r = self.new_reg()
                    self.emit(r + " = call i64 @" + ssa_sym + "(" + ", ".join(call_args) + ")", is_in_func)
                    return r
            func_reg = self.gen_expr(func_node, is_in_func)
            args_ptr = self.emit_args_array(arg_regs, is_in_func)
            r = self.new_reg()
            self.emit(r + " = call i64 @turbo_call(i64 " + func_reg + ", i32 " + str(argc) + ", i64* " + args_ptr + ")", is_in_func)
            return r
        elif node.type == "ATTR":
            obj_reg = self.gen_expr(node.children[0], is_in_func)
            attr_ptr = self.get_str_ptr(node.value, is_in_func)
            r = self.new_reg()
            self.emit(r + " = call i64 @turbo_getattr(i64 " + obj_reg + ", i8* " + attr_ptr + ")", is_in_func)
            return r
        elif node.type == "SUBSCRIPT":
            idx_node = node.children[1]
            if idx_node.type == "NAME" and idx_node.value in self.range_loop_vars:
                obj_reg = self.gen_expr(node.children[0], is_in_func)
                ri_loc = self.range_loop_vars[idx_node.value]
                r_idx = self.new_reg()
                self.emit(r_idx + " = load i64, i64* %loc_" + ri_loc, is_in_func)
                return self._emit_fast_list_getitem(obj_reg, r_idx, is_in_func)
            obj_reg = self.gen_expr(node.children[0], is_in_func)
            idx_reg = self.gen_expr(idx_node, is_in_func)
            r = self.new_reg()
            self.emit(r + " = call i64 @turbo_getitem(i64 " + obj_reg + ", i64 " + idx_reg + ")", is_in_func)
            return r
        elif node.type == "SLICE":
            obj_reg = self.gen_expr(node.children[0], is_in_func)
            start_reg = self.gen_expr(node.children[1], is_in_func)
            end_reg = self.gen_expr(node.children[2], is_in_func)
            r = self.new_reg()
            self.emit(r + " = call i64 @turbo_slice(i64 " + obj_reg + ", i64 " + start_reg + ", i64 " + end_reg + ")", is_in_func)
            return r
        elif node.type == "LIST":
            r_lst = self.new_reg()
            self.emit(r_lst + " = call i64 @make_list()", is_in_func)
            a_idx = 0
            while a_idx < len(node.children):
                elem_reg = self.gen_expr(node.children[a_idx], is_in_func)
                self.emit("call void @turbo_list_append(i64 " + r_lst + ", i64 " + elem_reg + ")", is_in_func)
                a_idx = a_idx + 1
            return r_lst
        elif node.type == "TUPLE":
            r_tup = self.new_reg()
            self.emit(r_tup + " = call i64 @make_tuple()", is_in_func)
            a_idx = 0
            while a_idx < len(node.children):
                elem_reg = self.gen_expr(node.children[a_idx], is_in_func)
                self.emit("call void @turbo_tuple_append(i64 " + r_tup + ", i64 " + elem_reg + ")", is_in_func)
                a_idx = a_idx + 1
            return r_tup
        elif node.type == "SET":
            r_set = self.new_reg()
            self.emit(r_set + " = call i64 @make_set()", is_in_func)
            a_idx = 0
            while a_idx < len(node.children):
                elem_reg = self.gen_expr(node.children[a_idx], is_in_func)
                self.emit("call void @turbo_set_add(i64 " + r_set + ", i64 " + elem_reg + ")", is_in_func)
                a_idx = a_idx + 1
            return r_set
        elif node.type == "DICT":
            keys = node.children[0].children
            values = node.children[1].children
            r_dict = self.new_reg()
            self.emit(r_dict + " = call i64 @make_dict()", is_in_func)
            a_idx = 0
            while a_idx < len(keys):
                k_reg = self.gen_expr(keys[a_idx], is_in_func)
                v_reg = self.gen_expr(values[a_idx], is_in_func)
                self.emit("call void @turbo_setitem(i64 " + r_dict + ", i64 " + k_reg + ", i64 " + v_reg + ")", is_in_func)
                a_idx = a_idx + 1
            return r_dict
        elif node.type == "LISTCOMP" or node.type == "GENCOMP":
            r_lc = self.new_reg()
            self.emit(r_lc + " = call i64 @make_list()", is_in_func)
            self._gen_comp_loops(node.children[1], r_lc, "LIST", node.children[0], None, is_in_func)
            return r_lc
        elif node.type == "SETCOMP":
            r_sc = self.new_reg()
            self.emit(r_sc + " = call i64 @make_set()", is_in_func)
            self._gen_comp_loops(node.children[1], r_sc, "SET", node.children[0], None, is_in_func)
            return r_sc
        elif node.type == "DICTCOMP":
            r_dc = self.new_reg()
            self.emit(r_dc + " = call i64 @make_dict()", is_in_func)
            self._gen_comp_loops(node.children[2], r_dc, "DICT", node.children[0], node.children[1], is_in_func)
            return r_dc
        elif node.type == "TERNARY":
            res_var = "_tern_" + str(self.reg_counter + 1)
            self.reg_counter = self.reg_counter + 1
            self.ensure_alloca(res_var, is_in_func)
            res_ptr = "%loc_" + res_var
            cond_reg = self.gen_expr(node.children[0], is_in_func)
            c_i1 = self.emit_truthy_i1(cond_reg, is_in_func)
            lbl_true = self.new_label("tern_true")
            lbl_false = self.new_label("tern_false")
            lbl_end = self.new_label("tern_end")
            self.emit_cond_br(c_i1, lbl_true, lbl_false, is_in_func)
            self.emit_label(lbl_true, is_in_func)
            tv = self.gen_expr(node.children[1], is_in_func)
            self.emit("store i64 " + tv + ", i64* " + res_ptr, is_in_func)
            self.emit_br(lbl_end, is_in_func)
            self.emit_label(lbl_false, is_in_func)
            fv = self.gen_expr(node.children[2], is_in_func)
            self.emit("store i64 " + fv + ", i64* " + res_ptr, is_in_func)
            self.emit_br(lbl_end, is_in_func)
            self.emit_label(lbl_end, is_in_func)
            r = self.new_reg()
            self.emit(r + " = load i64, i64* " + res_ptr, is_in_func)
            return r
        elif node.type == "ASSIGN":
            target = node.children[0]
            val_reg = self.gen_expr(node.children[1], is_in_func)
            if target.type == "NAME":
                self.emit_store_var(target.value, val_reg, is_in_func)
            return val_reg
        elif node.type == "LAMBDA":
            lambda_num = self.lambda_counter
            self.lambda_counter = self.lambda_counter + 1
            impl_name = "t_impl_lambda_" + str(lambda_num)
            params_node = node.children[0]
            body_node = node.children[1]

            saved_cur_lines = self.cur_lines
            saved_locals = self.local_vars
            saved_allocas = self.local_allocas
            saved_sized = self.sized_allocas
            saved_typed = self.typed_allocas
            saved_term = self.block_terminated
            saved_try = self.try_depth

            self.cur_lines = []
            self.local_vars = []
            self.local_allocas = []
            self.sized_allocas = []
            self.typed_allocas = []
            self.block_terminated = False
            self.try_depth = 0

            p_idx = 0
            while p_idx < len(params_node.children):
                p_name = params_node.children[p_idx].value
                self.local_vars.append(p_name)
                self.ensure_alloca(p_name, True)
                p_idx = p_idx + 1
            collect_locals(body_node, self.local_vars)
            l_idx = 0
            while l_idx < len(self.local_vars):
                self.ensure_alloca(self.local_vars[l_idx], True)
                l_idx = l_idx + 1

            p_idx = 0
            while p_idx < len(params_node.children):
                p_name = params_node.children[p_idx].value
                c_arg = self.new_reg()
                self.emit(c_arg + " = icmp sgt i32 %argc, " + str(p_idx), True)
                lbl_has = self.new_label("lam_arg_has")
                lbl_done = self.new_label("lam_arg_done")
                self.emit_cond_br(c_arg, lbl_has, lbl_done, True)
                self.emit_label(lbl_has, True)
                arg_ptr = self.new_reg()
                self.emit(arg_ptr + " = getelementptr inbounds i64, i64* %args, i64 " + str(p_idx), True)
                arg_val = self.new_reg()
                self.emit(arg_val + " = load i64, i64* " + arg_ptr, True)
                self.emit("store i64 " + arg_val + ", i64* %loc_" + p_name, True)
                self.emit_br(lbl_done, True)
                self.emit_label(lbl_done, True)
                p_idx = p_idx + 1

            body_reg = self.gen_expr(body_node, True)
            self.emit_ret(body_reg, True)

            fn_lines = ["define i64 @" + impl_name + "(i32 %argc, i64* %args) {", "entry:"]
            a_i = 0
            while a_i < len(self.local_allocas):
                aname = self.local_allocas[a_i]
                fn_lines.append("  %loc_" + aname + " = alloca i64")
                fn_lines.append("  store i64 -1688849860263936, i64* %loc_" + aname)
                a_i = a_i + 1
            a_i = 0
            while a_i < len(self.typed_allocas):
                tinfo = self.typed_allocas[a_i]
                fn_lines.append("  %loc_" + tinfo[0] + " = alloca " + tinfo[1])
                a_i = a_i + 1
            a_i = 0
            while a_i < len(self.sized_allocas):
                sinfo = self.sized_allocas[a_i]
                fn_lines.append("  %loc_" + sinfo[0] + " = alloca [" + str(sinfo[1]) + " x i64]")
                a_i = a_i + 1
            c_i = 0
            while c_i < len(self.cur_lines):
                fn_lines.append(self.cur_lines[c_i])
                c_i = c_i + 1
            fn_lines.append("}")
            self.funcs_list.append("\n".join(fn_lines))

            self.cur_lines = saved_cur_lines
            self.local_vars = saved_locals
            self.local_allocas = saved_allocas
            self.sized_allocas = saved_sized
            self.typed_allocas = saved_typed
            self.block_terminated = saved_term
            self.try_depth = saved_try

            sp = self.get_str_ptr("<lambda>", is_in_func)
            r = self.new_reg()
            self.emit(r + " = call i64 @make_func(i64 (i32, i64*)* @" + impl_name + ", i8* " + sp + ")", is_in_func)
            return r
        elif node.type == "YIELD":
            val_reg = self.gen_expr(node.children[0], is_in_func)
            yv = self.new_reg()
            self.emit(yv + " = load i64, i64* %loc___yield_values", is_in_func)
            self.emit("call void @turbo_list_append(i64 " + yv + ", i64 " + val_reg + ")", is_in_func)
            return "-1688849860263936"
        elif node.type == "YIELD_FROM":
            sub_reg = self.gen_expr(node.children[0], is_in_func)
            yv = self.new_reg()
            self.emit(yv + " = load i64, i64* %loc___yield_values", is_in_func)
            self._emit_iter_loop(sub_reg, "_yf_item", None, is_in_func, yv)
            return "-1688849860263936"
        elif node.type == "AWAIT":
            return self.gen_expr(node.children[0], is_in_func)
        else:
            print("CodeGen error: unknown expression " + node.type)
            return "-1688849860263936"

    def _emit_iter_loop(self, iter_reg, target_var_or_names, body_callback, is_in_func, yield_list_reg):
        # Iterate over a sequence in LLVM IR using direct %List / %String struct GEP access for lists/tuples!
        # Coerce iter_reg to a list/tuple or string; let's check tag via GEP or call @turbo_list(iter_reg) if not list/tuple!
        # Wait: calling @turbo_list(iter_reg) converts ANY iterable (list, tuple, str, dict, set) into a %List, or if it's already a %List/%Tuple we can GEP directly!
        # Even better: if we check if it's a pointer with tag 2 (%List) or 4 (%Tuple), we GEP %List directly; otherwise call @turbo_list to get a %List and GEP it!
        # Let's check: @turbo_list(i64 %v) returns a %List for list, tuple, str, set, dict!
        # Wait: if %v is already a %List (tag == 2) or %Tuple (tag == 4), we don't even need to copy it!
        lbl_is_seq = self.new_label("iter_seq")
        lbl_to_list = self.new_label("iter_conv")
        lbl_loop_init = self.new_label("iter_init")
        lbl_cond = self.new_label("iter_cond")
        lbl_body = self.new_label("iter_body")
        lbl_inc = self.new_label("iter_inc")
        lbl_end = self.new_label("iter_end")

        self.reg_counter = self.reg_counter + 1
        uid = str(self.reg_counter)
        seq_var = "_iter_seq_" + uid
        idx_var = "_iter_idx_" + uid
        len_var = "_iter_len_" + uid
        self.ensure_alloca(seq_var, is_in_func)
        self.ensure_alloca(idx_var, is_in_func)
        self.ensure_alloca(len_var, is_in_func)

        self.emit("store i64 " + iter_reg + ", i64* %loc_" + seq_var, is_in_func)
        # Check if pointer tag (0xFFFF000000000000 == 0xFFF9000000000000 -> -1970324836974592)
        r_tmask = self.new_reg()
        self.emit(r_tmask + " = and i64 " + iter_reg + ", -281474976710656", is_in_func)
        r_isptr = self.new_reg()
        self.emit(r_isptr + " = icmp eq i64 " + r_tmask + ", -1970324836974592", is_in_func)
        self.emit_cond_br(r_isptr, lbl_is_seq, lbl_to_list, is_in_func)

        self.emit_label(lbl_is_seq, is_in_func)
        r_raw = self.new_reg()
        self.emit(r_raw + " = and i64 " + iter_reg + ", 281474976710655", is_in_func)
        r_tagp = self.new_reg()
        self.emit(r_tagp + " = inttoptr i64 " + r_raw + " to i32*", is_in_func)
        r_tag = self.new_reg()
        self.emit(r_tag + " = load i32, i32* " + r_tagp, is_in_func)
        r_is_lst = self.new_reg()
        self.emit(r_is_lst + " = icmp eq i32 " + r_tag + ", 2", is_in_func)
        r_is_tup = self.new_reg()
        self.emit(r_is_tup + " = icmp eq i32 " + r_tag + ", 4", is_in_func)
        r_fast = self.new_reg()
        self.emit(r_fast + " = or i1 " + r_is_lst + ", " + r_is_tup, is_in_func)
        self.emit_cond_br(r_fast, lbl_loop_init, lbl_to_list, is_in_func)

        self.emit_label(lbl_to_list, is_in_func)
        p_arg = self.emit_args_array([iter_reg], is_in_func)
        r_conv = self.new_reg()
        self.emit(r_conv + " = call i64 @turbo_list(i32 1, i64* " + p_arg + ")", is_in_func)
        self.emit("store i64 " + r_conv + ", i64* %loc_" + seq_var, is_in_func)
        self.emit_br(lbl_loop_init, is_in_func)

        self.emit_label(lbl_loop_init, is_in_func)
        r_seq = self.new_reg()
        self.emit(r_seq + " = load i64, i64* %loc_" + seq_var, is_in_func)
        r_sraw = self.new_reg()
        self.emit(r_sraw + " = and i64 " + r_seq + ", 281474976710655", is_in_func)
        r_lptr = self.new_reg()
        self.emit(r_lptr + " = inttoptr i64 " + r_sraw + " to %List*", is_in_func)
        r_lenp = self.new_reg()
        self.emit(r_lenp + " = getelementptr inbounds %List, %List* " + r_lptr + ", i32 0, i32 1", is_in_func)
        r_len32 = self.new_reg()
        self.emit(r_len32 + " = load i32, i32* " + r_lenp, is_in_func)
        r_len64 = self.new_reg()
        self.emit(r_len64 + " = sext i32 " + r_len32 + " to i64", is_in_func)
        self.emit("store i64 " + r_len64 + ", i64* %loc_" + len_var, is_in_func)
        self.emit("store i64 0, i64* %loc_" + idx_var, is_in_func)
        self.emit_br(lbl_cond, is_in_func)

        self.emit_label(lbl_cond, is_in_func)
        r_cur_i = self.new_reg()
        self.emit(r_cur_i + " = load i64, i64* %loc_" + idx_var, is_in_func)
        # Re-read length in case list mutated or use stored length
        r_cur_seq = self.new_reg()
        self.emit(r_cur_seq + " = load i64, i64* %loc_" + seq_var, is_in_func)
        r_cur_raw = self.new_reg()
        self.emit(r_cur_raw + " = and i64 " + r_cur_seq + ", 281474976710655", is_in_func)
        r_cur_lptr = self.new_reg()
        self.emit(r_cur_lptr + " = inttoptr i64 " + r_cur_raw + " to %List*", is_in_func)
        r_cur_lenp = self.new_reg()
        self.emit(r_cur_lenp + " = getelementptr inbounds %List, %List* " + r_cur_lptr + ", i32 0, i32 1", is_in_func)
        r_cur_len32 = self.new_reg()
        self.emit(r_cur_len32 + " = load i32, i32* " + r_cur_lenp, is_in_func)
        r_cur_len64 = self.new_reg()
        self.emit(r_cur_len64 + " = sext i32 " + r_cur_len32 + " to i64", is_in_func)
        r_cmp = self.new_reg()
        self.emit(r_cmp + " = icmp slt i64 " + r_cur_i + ", " + r_cur_len64, is_in_func)
        self.emit_cond_br(r_cmp, lbl_body, lbl_end, is_in_func)

        self.emit_label(lbl_body, is_in_func)
        # Load items[i] via GEP on %List
        r_itemsp = self.new_reg()
        self.emit(r_itemsp + " = getelementptr inbounds %List, %List* " + r_cur_lptr + ", i32 0, i32 4", is_in_func)
        r_items = self.new_reg()
        self.emit(r_items + " = load i64*, i64** " + r_itemsp, is_in_func)
        r_elemp = self.new_reg()
        self.emit(r_elemp + " = getelementptr inbounds i64, i64* " + r_items + ", i64 " + r_cur_i, is_in_func)
        r_elem = self.new_reg()
        self.emit(r_elem + " = load i64, i64* " + r_elemp, is_in_func)

        if yield_list_reg != None:
            self.emit("call void @turbo_list_append(i64 " + yield_list_reg + ", i64 " + r_elem + ")", is_in_func)
        else:
            body_callback(r_elem, lbl_inc, lbl_end)

        self.emit_label(lbl_inc, is_in_func)
        r_i2 = self.new_reg()
        self.emit(r_i2 + " = load i64, i64* %loc_" + idx_var, is_in_func)
        r_inext = self.new_reg()
        self.emit(r_inext + " = add i64 " + r_i2 + ", 1", is_in_func)
        self.emit("store i64 " + r_inext + ", i64* %loc_" + idx_var, is_in_func)
        self.emit_br(lbl_cond, is_in_func)

        self.emit_label(lbl_end, is_in_func)

    def _emit_iter_header(self, iter_reg, is_in_func):
        lbl_is_seq = self.new_label("iter_seq")
        lbl_to_list = self.new_label("iter_conv")
        lbl_loop_init = self.new_label("iter_init")
        lbl_cond = self.new_label("iter_cond")
        lbl_body = self.new_label("iter_body")
        lbl_inc = self.new_label("iter_inc")
        lbl_end = self.new_label("iter_end")

        self.reg_counter = self.reg_counter + 1
        uid = str(self.reg_counter)
        seq_var = "_iter_seq_" + uid
        idx_var = "_iter_idx_" + uid
        self.ensure_alloca(seq_var, is_in_func)
        self.ensure_alloca(idx_var, is_in_func)

        self.emit("store i64 " + iter_reg + ", i64* %loc_" + seq_var, is_in_func)
        r_tmask = self.new_reg()
        self.emit(r_tmask + " = and i64 " + iter_reg + ", -281474976710656", is_in_func)
        r_isptr = self.new_reg()
        self.emit(r_isptr + " = icmp eq i64 " + r_tmask + ", -1970324836974592", is_in_func)
        self.emit_cond_br(r_isptr, lbl_is_seq, lbl_to_list, is_in_func)

        self.emit_label(lbl_is_seq, is_in_func)
        r_raw = self.new_reg()
        self.emit(r_raw + " = and i64 " + iter_reg + ", 281474976710655", is_in_func)
        r_tagp = self.new_reg()
        self.emit(r_tagp + " = inttoptr i64 " + r_raw + " to i32*", is_in_func)
        r_tag = self.new_reg()
        self.emit(r_tag + " = load i32, i32* " + r_tagp, is_in_func)
        r_is_lst = self.new_reg()
        self.emit(r_is_lst + " = icmp eq i32 " + r_tag + ", 2", is_in_func)
        r_is_tup = self.new_reg()
        self.emit(r_is_tup + " = icmp eq i32 " + r_tag + ", 4", is_in_func)
        r_fast = self.new_reg()
        self.emit(r_fast + " = or i1 " + r_is_lst + ", " + r_is_tup, is_in_func)
        self.emit_cond_br(r_fast, lbl_loop_init, lbl_to_list, is_in_func)

        self.emit_label(lbl_to_list, is_in_func)
        p_arg = self.emit_args_array([iter_reg], is_in_func)
        r_conv = self.new_reg()
        self.emit(r_conv + " = call i64 @turbo_list(i32 1, i64* " + p_arg + ")", is_in_func)
        self.emit("store i64 " + r_conv + ", i64* %loc_" + seq_var, is_in_func)
        self.emit_br(lbl_loop_init, is_in_func)

        self.emit_label(lbl_loop_init, is_in_func)
        self.emit("store i64 0, i64* %loc_" + idx_var, is_in_func)
        self.emit_br(lbl_cond, is_in_func)

        self.emit_label(lbl_cond, is_in_func)
        r_cur_i = self.new_reg()
        self.emit(r_cur_i + " = load i64, i64* %loc_" + idx_var, is_in_func)
        r_cur_seq = self.new_reg()
        self.emit(r_cur_seq + " = load i64, i64* %loc_" + seq_var, is_in_func)
        r_cur_raw = self.new_reg()
        self.emit(r_cur_raw + " = and i64 " + r_cur_seq + ", 281474976710655", is_in_func)
        r_cur_lptr = self.new_reg()
        self.emit(r_cur_lptr + " = inttoptr i64 " + r_cur_raw + " to %List*", is_in_func)
        r_cur_lenp = self.new_reg()
        self.emit(r_cur_lenp + " = getelementptr inbounds %List, %List* " + r_cur_lptr + ", i32 0, i32 1", is_in_func)
        r_cur_len32 = self.new_reg()
        self.emit(r_cur_len32 + " = load i32, i32* " + r_cur_lenp, is_in_func)
        r_cur_len64 = self.new_reg()
        self.emit(r_cur_len64 + " = sext i32 " + r_cur_len32 + " to i64", is_in_func)
        r_cmp = self.new_reg()
        self.emit(r_cmp + " = icmp slt i64 " + r_cur_i + ", " + r_cur_len64, is_in_func)
        self.emit_cond_br(r_cmp, lbl_body, lbl_end, is_in_func)

        self.emit_label(lbl_body, is_in_func)
        r_itemsp = self.new_reg()
        self.emit(r_itemsp + " = getelementptr inbounds %List, %List* " + r_cur_lptr + ", i32 0, i32 4", is_in_func)
        r_items = self.new_reg()
        self.emit(r_items + " = load i64*, i64** " + r_itemsp, is_in_func)
        r_elemp = self.new_reg()
        self.emit(r_elemp + " = getelementptr inbounds i64, i64* " + r_items + ", i64 " + r_cur_i, is_in_func)
        r_elem = self.new_reg()
        self.emit(r_elem + " = load i64, i64* " + r_elemp, is_in_func)
        return [r_elem, idx_var, lbl_cond, lbl_inc, lbl_end]

    def _emit_iter_footer(self, iter_state, is_in_func):
        idx_var = iter_state[1]
        lbl_cond = iter_state[2]
        lbl_inc = iter_state[3]
        lbl_end = iter_state[4]
        self.emit_label(lbl_inc, is_in_func)
        r_i2 = self.new_reg()
        self.emit(r_i2 + " = load i64, i64* %loc_" + idx_var, is_in_func)
        r_inext = self.new_reg()
        self.emit(r_inext + " = add i64 " + r_i2 + ", 1", is_in_func)
        self.emit("store i64 " + r_inext + ", i64* %loc_" + idx_var, is_in_func)
        self.emit_br(lbl_cond, is_in_func)
        self.emit_label(lbl_end, is_in_func)

    def _emit_fast_list_getitem(self, obj_reg, idx_i64_reg, is_in_func):
        r_raw = self.new_reg()
        self.emit(r_raw + " = and i64 " + obj_reg + ", 281474976710655", is_in_func)
        r_lptr = self.new_reg()
        self.emit(r_lptr + " = inttoptr i64 " + r_raw + " to %List*", is_in_func)
        r_itemsp = self.new_reg()
        self.emit(r_itemsp + " = getelementptr inbounds %List, %List* " + r_lptr + ", i32 0, i32 4", is_in_func)
        r_items = self.new_reg()
        self.emit(r_items + " = load i64*, i64** " + r_itemsp, is_in_func)
        r_elemp = self.new_reg()
        self.emit(r_elemp + " = getelementptr inbounds i64, i64* " + r_items + ", i64 " + idx_i64_reg, is_in_func)
        r_elem = self.new_reg()
        self.emit(r_elem + " = load i64, i64* " + r_elemp, is_in_func)
        return r_elem

    def _emit_fast_list_setitem(self, obj_reg, idx_i64_reg, val_reg, is_in_func):
        r_raw = self.new_reg()
        self.emit(r_raw + " = and i64 " + obj_reg + ", 281474976710655", is_in_func)
        r_lptr = self.new_reg()
        self.emit(r_lptr + " = inttoptr i64 " + r_raw + " to %List*", is_in_func)
        r_itemsp = self.new_reg()
        self.emit(r_itemsp + " = getelementptr inbounds %List, %List* " + r_lptr + ", i32 0, i32 4", is_in_func)
        r_items = self.new_reg()
        self.emit(r_items + " = load i64*, i64** " + r_itemsp, is_in_func)
        r_elemp = self.new_reg()
        self.emit(r_elemp + " = getelementptr inbounds i64, i64* " + r_items + ", i64 " + idx_i64_reg, is_in_func)
        self.emit("store i64 " + val_reg + ", i64* " + r_elemp, is_in_func)

    def _emit_fused_sum_comp(self, comp_node, is_in_func):
        elem_node = comp_node.children[0]
        gen_node = comp_node.children[1]
        # Fast path: single range(...) generator with no filter
        if (gen_node.type == "GENERATOR" or gen_node.type == "COMP_FOR") and gen_node.children[1].type == "CONST_NONE" and gen_node.children[2].type == "CONST_NONE":
            iter_node = gen_node.children[0]
            targets = gen_node.value.split(',')
            if len(targets) == 1 and iter_node.type == "CALL" and iter_node.children[0].type == "NAME" and iter_node.children[0].value == "range":
                rargs = iter_node.children[1].children
                if len(rargs) >= 1 and len(rargs) <= 2:
                    k_var = targets[0]
                    if is_in_func:
                        if not self.is_local(k_var, True):
                            self.local_vars.append(k_var)
                        self.ensure_alloca(k_var, True)
                    else:
                        self.add_global(k_var)
                    r_start = "0"
                    r_stop = ""
                    if len(rargs) == 1:
                        r_stop = self._gen_range_bound(rargs[0], is_in_func)
                    else:
                        r_start = self._gen_range_bound(rargs[0], is_in_func)
                        r_stop = self._gen_range_bound(rargs[1], is_in_func)
                    self.reg_counter = self.reg_counter + 1
                    uid = str(self.reg_counter)
                    k_loc = "_sum_k_" + uid
                    acc_loc = "_sum_acc_" + uid
                    self.ensure_typed_alloca(k_loc, "i64", is_in_func)
                    # Check if elem_node is a product of two 2D subscripts (e.g. A[i][k] * B[k][j])
                    is_2d_prod = False
                    if elem_node.type == "BINOP" and elem_node.value == "*":
                        lhs_e = elem_node.children[0]
                        rhs_e = elem_node.children[1]
                        if lhs_e.type == "SUBSCRIPT" and lhs_e.children[0].type == "SUBSCRIPT" and rhs_e.type == "SUBSCRIPT" and rhs_e.children[0].type == "SUBSCRIPT":
                            is_2d_prod = True
                    if is_2d_prod:
                        self.ensure_typed_alloca(acc_loc, "double", is_in_func)
                        self.emit("store double 0.0, double* %loc_" + acc_loc, is_in_func)
                        self.emit("store i64 " + r_start + ", i64* %loc_" + k_loc, is_in_func)
                        saved_k = None
                        if k_var in self.range_loop_vars:
                            saved_k = self.range_loop_vars[k_var]
                        self.range_loop_vars[k_var] = k_loc
                        lhs_e = elem_node.children[0]
                        rhs_e = elem_node.children[1]
                        # Hoist loop-invariant outer subscripts out of the k-loop
                        lhs_outer_k = (lhs_e.children[0].children[1].type == "NAME" and lhs_e.children[0].children[1].value == k_var)
                        rhs_outer_k = (rhs_e.children[0].children[1].type == "NAME" and rhs_e.children[0].children[1].value == k_var)
                        lhs_row_items = ""
                        rhs_outer_items = ""
                        if not lhs_outer_k:
                            lhs_row_tv = self.gen_expr(lhs_e.children[0], is_in_func)
                            lr_raw = self.new_reg()
                            self.emit(lr_raw + " = and i64 " + lhs_row_tv + ", 281474976710655", is_in_func)
                            lr_ptr = self.new_reg()
                            self.emit(lr_ptr + " = inttoptr i64 " + lr_raw + " to %List*", is_in_func)
                            lr_ip = self.new_reg()
                            self.emit(lr_ip + " = getelementptr inbounds %List, %List* " + lr_ptr + ", i32 0, i32 4", is_in_func)
                            lhs_row_items = self.new_reg()
                            self.emit(lhs_row_items + " = load i64*, i64** " + lr_ip, is_in_func)
                        if rhs_outer_k:
                            rhs_mat_tv = self.gen_expr(rhs_e.children[0].children[0], is_in_func)
                            rm_raw = self.new_reg()
                            self.emit(rm_raw + " = and i64 " + rhs_mat_tv + ", 281474976710655", is_in_func)
                            rm_ptr = self.new_reg()
                            self.emit(rm_ptr + " = inttoptr i64 " + rm_raw + " to %List*", is_in_func)
                            rm_ip = self.new_reg()
                            self.emit(rm_ip + " = getelementptr inbounds %List, %List* " + rm_ptr + ", i32 0, i32 4", is_in_func)
                            rhs_outer_items = self.new_reg()
                            self.emit(rhs_outer_items + " = load i64*, i64** " + rm_ip, is_in_func)
                        lbl_cond = self.new_label("fsum_cond")
                        lbl_body = self.new_label("fsum_body")
                        lbl_end = self.new_label("fsum_end")
                        self.emit_label(lbl_cond, is_in_func)
                        rk = self.new_reg()
                        self.emit(rk + " = load i64, i64* %loc_" + k_loc, is_in_func)
                        rcmp = self.new_reg()
                        self.emit(rcmp + " = icmp slt i64 " + rk + ", " + r_stop, is_in_func)
                        self.emit_cond_br(rcmp, lbl_body, lbl_end, is_in_func)
                        self.emit_label(lbl_body, is_in_func)
                        if lhs_row_items != "" and lhs_e.children[1].type == "NAME" and lhs_e.children[1].value == k_var:
                            lep = self.new_reg()
                            self.emit(lep + " = getelementptr inbounds i64, i64* " + lhs_row_items + ", i64 " + rk, is_in_func)
                            lv_tv = self.new_reg()
                            self.emit(lv_tv + " = load i64, i64* " + lep, is_in_func)
                        else:
                            lv_tv = self.gen_expr(lhs_e, is_in_func)
                        if rhs_outer_items != "" and rhs_e.children[1].type == "NAME" and rhs_e.children[1].value in self.range_loop_vars:
                            r_krow_p = self.new_reg()
                            self.emit(r_krow_p + " = getelementptr inbounds i64, i64* " + rhs_outer_items + ", i64 " + rk, is_in_func)
                            r_krow_tv = self.new_reg()
                            self.emit(r_krow_tv + " = load i64, i64* " + r_krow_p, is_in_func)
                            j_loc = self.range_loop_vars[rhs_e.children[1].value]
                            rj = self.new_reg()
                            self.emit(rj + " = load i64, i64* %loc_" + j_loc, is_in_func)
                            rv_tv = self._emit_fast_list_getitem(r_krow_tv, rj, is_in_func)
                        else:
                            rv_tv = self.gen_expr(rhs_e, is_in_func)
                        lv_d = self.new_reg()
                        self.emit(lv_d + " = bitcast i64 " + lv_tv + " to double", is_in_func)
                        rv_d = self.new_reg()
                        self.emit(rv_d + " = bitcast i64 " + rv_tv + " to double", is_in_func)
                        prod_d = self.new_reg()
                        self.emit(prod_d + " = fmul double " + lv_d + ", " + rv_d, is_in_func)
                        cur_acc = self.new_reg()
                        self.emit(cur_acc + " = load double, double* %loc_" + acc_loc, is_in_func)
                        next_acc = self.new_reg()
                        self.emit(next_acc + " = fadd double " + cur_acc + ", " + prod_d, is_in_func)
                        self.emit("store double " + next_acc + ", double* %loc_" + acc_loc, is_in_func)
                        rk2 = self.new_reg()
                        self.emit(rk2 + " = load i64, i64* %loc_" + k_loc, is_in_func)
                        rkn = self.new_reg()
                        self.emit(rkn + " = add i64 " + rk2 + ", 1", is_in_func)
                        self.emit("store i64 " + rkn + ", i64* %loc_" + k_loc, is_in_func)
                        self.emit_br(lbl_cond, is_in_func)
                        self.emit_label(lbl_end, is_in_func)
                        if saved_k != None:
                            self.range_loop_vars[k_var] = saved_k
                        else:
                            del self.range_loop_vars[k_var]
                        fin_d = self.new_reg()
                        self.emit(fin_d + " = load double, double* %loc_" + acc_loc, is_in_func)
                        return self.emit_box_float(fin_d, is_in_func)
                    else:
                        self.ensure_alloca(acc_loc, is_in_func)
                        self.emit("store i64 -2251799813685248, i64* %loc_" + acc_loc, is_in_func)
                        self.emit("store i64 " + r_start + ", i64* %loc_" + k_loc, is_in_func)
                        saved_k = None
                        if k_var in self.range_loop_vars:
                            saved_k = self.range_loop_vars[k_var]
                        self.range_loop_vars[k_var] = k_loc
                        lbl_cond = self.new_label("fsum_cond")
                        lbl_body = self.new_label("fsum_body")
                        lbl_end = self.new_label("fsum_end")
                        self.emit_label(lbl_cond, is_in_func)
                        rk = self.new_reg()
                        self.emit(rk + " = load i64, i64* %loc_" + k_loc, is_in_func)
                        rcmp = self.new_reg()
                        self.emit(rcmp + " = icmp slt i64 " + rk + ", " + r_stop, is_in_func)
                        self.emit_cond_br(rcmp, lbl_body, lbl_end, is_in_func)
                        self.emit_label(lbl_body, is_in_func)
                        rk_box = self.emit_box_int(rk, is_in_func)
                        self.emit_store_var(k_var, rk_box, is_in_func)
                        ev = self.gen_expr(elem_node, is_in_func)
                        cur_acc = self.new_reg()
                        self.emit(cur_acc + " = load i64, i64* %loc_" + acc_loc, is_in_func)
                        next_acc = self.new_reg()
                        self.emit(next_acc + " = call i64 @turbo_add(i64 " + cur_acc + ", i64 " + ev + ")", is_in_func)
                        self.emit("store i64 " + next_acc + ", i64* %loc_" + acc_loc, is_in_func)
                        rk2 = self.new_reg()
                        self.emit(rk2 + " = load i64, i64* %loc_" + k_loc, is_in_func)
                        rkn = self.new_reg()
                        self.emit(rkn + " = add i64 " + rk2 + ", 1", is_in_func)
                        self.emit("store i64 " + rkn + ", i64* %loc_" + k_loc, is_in_func)
                        self.emit_br(lbl_cond, is_in_func)
                        self.emit_label(lbl_end, is_in_func)
                        if saved_k != None:
                            self.range_loop_vars[k_var] = saved_k
                        else:
                            del self.range_loop_vars[k_var]
                        fin_v = self.new_reg()
                        self.emit(fin_v + " = load i64, i64* %loc_" + acc_loc, is_in_func)
                        return fin_v
        # General fallback: accumulate in-place without allocating a list
        self.reg_counter = self.reg_counter + 1
        acc_var = "_gsum_acc_" + str(self.reg_counter)
        self.ensure_alloca(acc_var, is_in_func)
        self.emit("store i64 -2251799813685248, i64* %loc_" + acc_var, is_in_func)
        self._gen_comp_loops(gen_node, "%loc_" + acc_var, "SUM", elem_node, None, is_in_func)
        r_res = self.new_reg()
        self.emit(r_res + " = load i64, i64* %loc_" + acc_var, is_in_func)
        return r_res

    def _gen_comp_loops(self, gen_node, container_reg, comp_kind, elem_node, val_node, is_in_func):
        if gen_node.type == "CONST_NONE":
            if comp_kind == "LIST":
                er = self.gen_expr(elem_node, is_in_func)
                self.emit("call void @turbo_list_append(i64 " + container_reg + ", i64 " + er + ")", is_in_func)
            elif comp_kind == "SUM":
                er = self.gen_expr(elem_node, is_in_func)
                cur_a = self.new_reg()
                self.emit(cur_a + " = load i64, i64* " + container_reg, is_in_func)
                nxt_a = self.new_reg()
                self.emit(nxt_a + " = call i64 @turbo_add(i64 " + cur_a + ", i64 " + er + ")", is_in_func)
                self.emit("store i64 " + nxt_a + ", i64* " + container_reg, is_in_func)
            elif comp_kind == "SET":
                er = self.gen_expr(elem_node, is_in_func)
                self.emit("call void @turbo_set_add(i64 " + container_reg + ", i64 " + er + ")", is_in_func)
            elif comp_kind == "DICT":
                kr = self.gen_expr(elem_node, is_in_func)
                vr = self.gen_expr(val_node, is_in_func)
                self.emit("call void @turbo_setitem(i64 " + container_reg + ", i64 " + kr + ", i64 " + vr + ")", is_in_func)
            return

        targets = gen_node.value.split(',')
        # Ensure target variables are in local_vars if in func, or global_vars if in main
        t_i = 0
        while t_i < len(targets):
            tname = targets[t_i]
            if is_in_func:
                if not self.is_local(tname, True):
                    self.local_vars.append(tname)
                self.ensure_alloca(tname, True)
            else:
                self.add_global(tname)
            t_i = t_i + 1

        iter_node = gen_node.children[0]
        if len(targets) == 1 and iter_node.type == "CALL" and iter_node.children[0].type == "NAME" and iter_node.children[0].value == "range":
            rargs = iter_node.children[1].children
            if len(rargs) >= 1 and len(rargs) <= 2:
                r_start = "0"
                r_stop = ""
                if len(rargs) == 1:
                    r_stop = self._gen_range_bound(rargs[0], is_in_func)
                else:
                    r_start = self._gen_range_bound(rargs[0], is_in_func)
                    r_stop = self._gen_range_bound(rargs[1], is_in_func)
                self.reg_counter = self.reg_counter + 1
                ri_var = "_comp_rng_" + str(self.reg_counter)
                self.ensure_typed_alloca(ri_var, "i64", is_in_func)
                self.emit("store i64 " + r_start + ", i64* %loc_" + ri_var, is_in_func)
                lbl_cond = self.new_label("crng_cond")
                lbl_body = self.new_label("crng_body")
                lbl_inc = self.new_label("crng_inc")
                lbl_end = self.new_label("crng_end")
                self.emit_label(lbl_cond, is_in_func)
                r_ri = self.new_reg()
                self.emit(r_ri + " = load i64, i64* %loc_" + ri_var, is_in_func)
                r_cmp = self.new_reg()
                self.emit(r_cmp + " = icmp slt i64 " + r_ri + ", " + r_stop, is_in_func)
                self.emit_cond_br(r_cmp, lbl_body, lbl_end, is_in_func)
                self.emit_label(lbl_body, is_in_func)
                t_var = targets[0]
                r_boxed = self.emit_box_int(r_ri, is_in_func)
                self.emit_store_var(t_var, r_boxed, is_in_func)
                saved_rlv = None
                if t_var in self.range_loop_vars:
                    saved_rlv = self.range_loop_vars[t_var]
                self.range_loop_vars[t_var] = ri_var
                if_cond = gen_node.children[1]
                if if_cond.type != "CONST_NONE":
                    cond_reg = self.gen_expr(if_cond, is_in_func)
                    c_i1 = self.emit_truthy_i1(cond_reg, is_in_func)
                    lbl_pass = self.new_label("comp_if_pass")
                    self.emit_cond_br(c_i1, lbl_pass, lbl_inc, is_in_func)
                    self.emit_label(lbl_pass, is_in_func)
                next_gen = gen_node.children[2]
                self._gen_comp_loops(next_gen, container_reg, comp_kind, elem_node, val_node, is_in_func)
                if saved_rlv != None:
                    self.range_loop_vars[t_var] = saved_rlv
                elif t_var in self.range_loop_vars:
                    del self.range_loop_vars[t_var]
                self.emit_label(lbl_inc, is_in_func)
                r_ri2 = self.new_reg()
                self.emit(r_ri2 + " = load i64, i64* %loc_" + ri_var, is_in_func)
                r_rinext = self.new_reg()
                self.emit(r_rinext + " = add i64 " + r_ri2 + ", 1", is_in_func)
                self.emit("store i64 " + r_rinext + ", i64* %loc_" + ri_var, is_in_func)
                self.emit_br(lbl_cond, is_in_func)
                self.emit_label(lbl_end, is_in_func)
                return

        iter_reg = self.gen_expr(iter_node, is_in_func)
        iter_state = self._emit_iter_header(iter_reg, is_in_func)
        r_elem = iter_state[0]
        lbl_inc = iter_state[3]

        if len(targets) == 1:
            self.emit_store_var(targets[0], r_elem, is_in_func)
        else:
            u_i = 0
            while u_i < len(targets):
                idx_const = str(-2251799813685248 + u_i)
                r_sub = self.new_reg()
                self.emit(r_sub + " = call i64 @turbo_getitem(i64 " + r_elem + ", i64 " + idx_const + ")", is_in_func)
                self.emit_store_var(targets[u_i], r_sub, is_in_func)
                u_i = u_i + 1

        if_cond = gen_node.children[1]
        if if_cond.type != "CONST_NONE":
            cond_reg = self.gen_expr(if_cond, is_in_func)
            c_i1 = self.emit_truthy_i1(cond_reg, is_in_func)
            lbl_pass = self.new_label("comp_if_pass")
            self.emit_cond_br(c_i1, lbl_pass, lbl_inc, is_in_func)
            self.emit_label(lbl_pass, is_in_func)

        next_gen = gen_node.children[2]
        self._gen_comp_loops(next_gen, container_reg, comp_kind, elem_node, val_node, is_in_func)
        self._emit_iter_footer(iter_state, is_in_func)

    def _is_raw_int_expr(self, node, param_names, func_name):
        if node.type == "CONST_INT":
            return True
        if node.type == "NAME":
            i = 0
            while i < len(param_names):
                if param_names[i] == node.value:
                    return True
                i = i + 1
            return False
        if node.type == "UNARY" and node.value == "-":
            return self._is_raw_int_expr(node.children[0], param_names, func_name)
        if node.type == "BINOP":
            if not (node.value in ("+", "-", "*", "//", "%")):
                return False
            return self._is_raw_int_expr(node.children[0], param_names, func_name) and self._is_raw_int_expr(node.children[1], param_names, func_name)
        if node.type == "COMP":
            if not (node.value in ("<", "<=", ">", ">=", "==", "!=")):
                return False
            return self._is_raw_int_expr(node.children[0], param_names, func_name) and self._is_raw_int_expr(node.children[1], param_names, func_name)
        if node.type == "CALL":
            fn = node.children[0]
            args = node.children[1].children
            if fn.type == "NAME" and fn.value == func_name and len(args) == len(param_names):
                i = 0
                while i < len(args):
                    if not self._is_raw_int_expr(args[i], param_names, func_name):
                        return False
                    i = i + 1
                return True
        return False

    def _is_raw_int_block(self, block_node, param_names, func_name):
        i = 0
        while i < len(block_node.children):
            stmt = block_node.children[i]
            if stmt.type == "RETURN":
                if not self._is_raw_int_expr(stmt.children[0], param_names, func_name):
                    return False
            elif stmt.type == "IF":
                if not self._is_raw_int_expr(stmt.children[0], param_names, func_name):
                    return False
                if not self._is_raw_int_block(stmt.children[1], param_names, func_name):
                    return False
                if len(stmt.children) >= 3:
                    else_n = stmt.children[2]
                    if else_n.type == "IF":
                        if not self._is_raw_int_block(ASTNode("BLOCK", "", [else_n]), param_names, func_name):
                            return False
                    else:
                        if not self._is_raw_int_block(else_n, param_names, func_name):
                            return False
            else:
                return False
            i = i + 1
        return True

    def _emit_raw_int_expr(self, node, param_map, func_name, raw_sym):
        if node.type == "CONST_INT":
            return str(int(node.value))
        if node.type == "NAME":
            return param_map[node.value]
        if node.type == "UNARY" and node.value == "-":
            v = self._emit_raw_int_expr(node.children[0], param_map, func_name, raw_sym)
            r = self.new_reg()
            self.emit(r + " = sub i64 0, " + v, True)
            return r
        if node.type == "BINOP":
            lv = self._emit_raw_int_expr(node.children[0], param_map, func_name, raw_sym)
            rv = self._emit_raw_int_expr(node.children[1], param_map, func_name, raw_sym)
            op = node.value
            iop = "add"
            if op == "-":
                iop = "sub"
            elif op == "*":
                iop = "mul"
            elif op == "//":
                iop = "sdiv"
            elif op == "%":
                iop = "srem"
            r = self.new_reg()
            self.emit(r + " = " + iop + " i64 " + lv + ", " + rv, True)
            return r
        if node.type == "CALL":
            args = node.children[1].children
            cargs = []
            i = 0
            while i < len(args):
                av = self._emit_raw_int_expr(args[i], param_map, func_name, raw_sym)
                cargs.append("i64 " + av)
                i = i + 1
            r = self.new_reg()
            self.emit(r + " = call i64 @" + raw_sym + "(" + ", ".join(cargs) + ")", True)
            return r
        return "0"

    def _emit_raw_int_cond(self, node, param_map, func_name, raw_sym):
        if node.type == "COMP":
            lv = self._emit_raw_int_expr(node.children[0], param_map, func_name, raw_sym)
            rv = self._emit_raw_int_expr(node.children[1], param_map, func_name, raw_sym)
            op = node.value
            cop = "eq"
            if op == "<":
                cop = "slt"
            elif op == "<=":
                cop = "sle"
            elif op == ">":
                cop = "sgt"
            elif op == ">=":
                cop = "sge"
            elif op == "!=":
                cop = "ne"
            r = self.new_reg()
            self.emit(r + " = icmp " + cop + " i64 " + lv + ", " + rv, True)
            return r
        v = self._emit_raw_int_expr(node, param_map, func_name, raw_sym)
        r = self.new_reg()
        self.emit(r + " = icmp ne i64 " + v + ", 0", True)
        return r

    def _emit_raw_int_block(self, block_node, param_map, func_name, raw_sym):
        i = 0
        while i < len(block_node.children):
            stmt = block_node.children[i]
            if stmt.type == "RETURN":
                rv = self._emit_raw_int_expr(stmt.children[0], param_map, func_name, raw_sym)
                self.emit_ret(rv, True)
            elif stmt.type == "IF":
                c_i1 = self._emit_raw_int_cond(stmt.children[0], param_map, func_name, raw_sym)
                lbl_then = self.new_label("raw_then")
                lbl_else = self.new_label("raw_else")
                lbl_end = self.new_label("raw_end")
                if len(stmt.children) >= 3:
                    self.emit_cond_br(c_i1, lbl_then, lbl_else, True)
                    self.emit_label(lbl_then, True)
                    self._emit_raw_int_block(stmt.children[1], param_map, func_name, raw_sym)
                    self.emit_br(lbl_end, True)
                    self.emit_label(lbl_else, True)
                    else_n = stmt.children[2]
                    if else_n.type == "IF":
                        self._emit_raw_int_block(ASTNode("BLOCK", "", [else_n]), param_map, func_name, raw_sym)
                    else:
                        self._emit_raw_int_block(else_n, param_map, func_name, raw_sym)
                    self.emit_br(lbl_end, True)
                    self.emit_label(lbl_end, True)
                else:
                    self.emit_cond_br(c_i1, lbl_then, lbl_end, True)
                    self.emit_label(lbl_then, True)
                    self._emit_raw_int_block(stmt.children[1], param_map, func_name, raw_sym)
                    self.emit_br(lbl_end, True)
                    self.emit_label(lbl_end, True)
            i = i + 1

    def gen_func_def(self, node, class_name, mod_prefix, is_nested):
        func_name = node.value
        if class_name != "":
            impl_name = "t_impl_" + class_name + "_" + func_name
            ssa_name = "t_ssa_" + class_name + "_" + func_name
        else:
            impl_name = "t_impl_" + mod_prefix + func_name
            ssa_name = "t_ssa_" + mod_prefix + func_name
        var_name = func_name

        params_node = node.children[0]
        body_node = node.children[1]
        num_params = len(params_node.children)

        # Register for direct SSA call if top-level or nested without decorators
        has_decorators = False
        d_chk = 2
        while d_chk < len(node.children):
            if node.children[d_chk].type == "DECORATOR":
                has_decorators = True
            d_chk = d_chk + 1
        if class_name == "" and not has_decorators:
            self.direct_funcs[mod_prefix + func_name] = [ssa_name, num_params]

        saved_cur_lines = self.cur_lines
        saved_locals = self.local_vars
        saved_allocas = self.local_allocas
        saved_sized = self.sized_allocas
        saved_typed = self.typed_allocas
        saved_term = self.block_terminated
        saved_try = self.try_depth
        saved_loop = self.loop_stack
        saved_gen = self.is_generator
        saved_pnv = self.pending_native_vars
        saved_vt = self._var_types
        saved_mp = self.mod_prefix
        saved_ldf = self.local_direct_funcs
        saved_rlv = self.range_loop_vars

        self.cur_lines = []
        self.local_vars = []
        self.local_allocas = []
        self.sized_allocas = []
        self.typed_allocas = []
        self.block_terminated = False
        self.try_depth = 0
        self.loop_stack = []
        self.pending_native_vars = None
        self.local_direct_funcs = {}
        self.range_loop_vars = {}
        if class_name == "" and is_nested and not has_decorators:
            self.local_direct_funcs[func_name] = mod_prefix + func_name
        if mod_prefix != "" and class_name == "" and not is_nested:
            self.mod_prefix = mod_prefix

        p_idx = 0
        while p_idx < num_params:
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
                else:
                    self.add_global(l_name)
                l_idx = l_idx + 1
            self.local_vars = new_locals

        l_idx = 0
        while l_idx < len(self.local_vars):
            self.ensure_alloca(self.local_vars[l_idx], True)
            l_idx = l_idx + 1

        generator_func = has_yield(body_node)
        self.is_generator = generator_func
        if generator_func:
            self.ensure_alloca("__yield_values", True)
            r_yl = self.new_reg()
            self.emit(r_yl + " = call i64 @make_list()", True)
            self.emit("store i64 " + r_yl + ", i64* %loc___yield_values", True)

        # Extract nested function defs
        nested_funcs = []
        remaining_stmts = []
        s_i = 0
        while s_i < len(body_node.children):
            stmt = body_node.children[s_i]
            if stmt.type == "DEF" or stmt.type == "ASYNC_DEF":
                nested_funcs.append(stmt)
            else:
                remaining_stmts.append(stmt)
            s_i = s_i + 1

        nf_i = 0
        while nf_i < len(nested_funcs):
            nf = nested_funcs[nf_i]
            nested_mod_prefix = mod_prefix + func_name + "_nested_"
            self.gen_func_def(nf, "", nested_mod_prefix, True)
            self.local_direct_funcs[nf.value] = nested_mod_prefix + nf.value
            self.add_global(nf.value)
            sp_nf = self.get_str_ptr(nf.value, True)
            r_nf = self.new_reg()
            self.emit(r_nf + " = call i64 @make_func(i64 (i32, i64*)* @t_impl_" + nested_mod_prefix + nf.value + ", i8* " + sp_nf + ")", True)
            self.emit("store i64 " + r_nf + ", i64* @t_" + nf.value, True)
            nf_i = nf_i + 1
        body_node.children = remaining_stmts

        # Check if function is a pure raw-int recursive/numeric function
        param_names = []
        p_idx = 0
        while p_idx < num_params:
            param_names.append(params_node.children[p_idx].value)
            p_idx = p_idx + 1
        is_raw_int = False
        raw_name = "t_raw_" + mod_prefix + func_name
        if class_name == "" and not has_decorators and not generator_func and len(nested_funcs) == 0 and num_params > 0 and len(body_node.children) > 0:
            if self._is_raw_int_block(body_node, param_names, func_name):
                is_raw_int = True
                self.raw_int_funcs[mod_prefix + func_name] = [raw_name, num_params]

        if is_raw_int:
            raw_cur_lines = self.cur_lines
            raw_term = self.block_terminated
            self.cur_lines = []
            self.block_terminated = False
            raw_param_map = {}
            raw_sig_parts = []
            p_idx = 0
            while p_idx < num_params:
                raw_param_map[param_names[p_idx]] = "%rarg_" + str(p_idx)
                raw_sig_parts.append("i64 %rarg_" + str(p_idx))
                p_idx = p_idx + 1
            self._emit_raw_int_block(body_node, raw_param_map, func_name, raw_name)
            if not self.block_terminated:
                self.emit_ret("0", True)
            raw_lines = ["define i64 @" + raw_name + "(" + ", ".join(raw_sig_parts) + ") {", "entry:"]
            c_i = 0
            while c_i < len(self.cur_lines):
                raw_lines.append(self.cur_lines[c_i])
                c_i = c_i + 1
            raw_lines.append("}")
            self.funcs_list.append("\n".join(raw_lines))

            self.cur_lines = raw_cur_lines
            self.block_terminated = raw_term
            unboxed_args = []
            p_idx = 0
            while p_idx < num_params:
                u_reg = self.emit_unbox_int("%arg_" + str(p_idx), True)
                unboxed_args.append("i64 " + u_reg)
                p_idx = p_idx + 1
            r_raw_res = self.new_reg()
            self.emit(r_raw_res + " = call i64 @" + raw_name + "(" + ", ".join(unboxed_args) + ")", True)
            r_box_res = self.emit_box_int(r_raw_res, True)
            self.emit_ret(r_box_res, True)
        else:
            self._var_types = {}
            self._infer_func_types(body_node, self._var_types)

            self.gen_block_stmts(body_node, True)

            if not self.block_terminated:
                if generator_func:
                    r_yv = self.new_reg()
                    self.emit(r_yv + " = load i64, i64* %loc___yield_values", True)
                    self.emit_ret(r_yv, True)
                else:
                    self.emit_ret("-1688849860263936", True)

        # Build SSA function definition
        param_sig_parts = []
        p_idx = 0
        while p_idx < num_params:
            param_sig_parts.append("i64 %arg_" + str(p_idx))
            p_idx = p_idx + 1
        fn_lines = ["define i64 @" + ssa_name + "(" + ", ".join(param_sig_parts) + ") {", "entry:"]
        a_i = 0
        while a_i < len(self.local_allocas):
            aname = self.local_allocas[a_i]
            fn_lines.append("  %loc_" + aname + " = alloca i64")
            fn_lines.append("  store i64 -1688849860263936, i64* %loc_" + aname)
            a_i = a_i + 1
        a_i = 0
        while a_i < len(self.typed_allocas):
            tinfo = self.typed_allocas[a_i]
            fn_lines.append("  %loc_" + tinfo[0] + " = alloca " + tinfo[1])
            a_i = a_i + 1
        a_i = 0
        while a_i < len(self.sized_allocas):
            sinfo = self.sized_allocas[a_i]
            fn_lines.append("  %loc_" + sinfo[0] + " = alloca [" + str(sinfo[1]) + " x i64]")
            a_i = a_i + 1
        p_idx = 0
        while p_idx < num_params:
            p_name = params_node.children[p_idx].value
            fn_lines.append("  store i64 %arg_" + str(p_idx) + ", i64* %loc_" + p_name)
            p_idx = p_idx + 1
        c_i = 0
        while c_i < len(self.cur_lines):
            fn_lines.append(self.cur_lines[c_i])
            c_i = c_i + 1
        fn_lines.append("}")
        self.funcs_list.append("\n".join(fn_lines))

        # Build dynamic dispatch wrapper thunk @impl_name(i32 %argc, i64* %args) -> calls @ssa_name
        w_lines = ["define i64 @" + impl_name + "(i32 %argc, i64* %args) {", "entry:"]
        w_call_args = []
        p_idx = 0
        while p_idx < num_params:
            w_lines.append("  %w_slot_" + str(p_idx) + " = alloca i64")
            w_lines.append("  store i64 -1688849860263936, i64* %w_slot_" + str(p_idx))
            w_lines.append("  %w_has_" + str(p_idx) + " = icmp sgt i32 %argc, " + str(p_idx))
            w_lines.append("  br i1 %w_has_" + str(p_idx) + ", label %w_ld_" + str(p_idx) + ", label %w_nx_" + str(p_idx))
            w_lines.append("w_ld_" + str(p_idx) + ":")
            w_lines.append("  %w_ptr_" + str(p_idx) + " = getelementptr inbounds i64, i64* %args, i64 " + str(p_idx))
            w_lines.append("  %w_val_" + str(p_idx) + " = load i64, i64* %w_ptr_" + str(p_idx))
            w_lines.append("  store i64 %w_val_" + str(p_idx) + ", i64* %w_slot_" + str(p_idx))
            w_lines.append("  br label %w_nx_" + str(p_idx))
            w_lines.append("w_nx_" + str(p_idx) + ":")
            w_lines.append("  %w_arg_" + str(p_idx) + " = load i64, i64* %w_slot_" + str(p_idx))
            w_call_args.append("i64 %w_arg_" + str(p_idx))
            p_idx = p_idx + 1
        w_lines.append("  %w_ret = call i64 @" + ssa_name + "(" + ", ".join(w_call_args) + ")")
        w_lines.append("  ret i64 %w_ret")
        w_lines.append("}")
        self.funcs_list.append("\n".join(w_lines))

        self.cur_lines = saved_cur_lines
        self.local_vars = saved_locals
        self.local_allocas = saved_allocas
        self.sized_allocas = saved_sized
        self.typed_allocas = saved_typed
        self.block_terminated = saved_term
        self.try_depth = saved_try
        self.loop_stack = saved_loop
        self.is_generator = saved_gen
        self.pending_native_vars = saved_pnv
        self._var_types = saved_vt
        self.mod_prefix = saved_mp
        self.local_direct_funcs = saved_ldf
        self.range_loop_vars = saved_rlv

        if class_name == "":
            self.add_global(mod_prefix + var_name)
            if not is_nested and mod_prefix == "":
                sp_fn = self.get_str_ptr(func_name, False)
                r_fn = self.new_reg()
                self.emit(r_fn + " = call i64 @make_func(i64 (i32, i64*)* @" + impl_name + ", i8* " + sp_fn + ")", False)
                self.emit("store i64 " + r_fn + ", i64* @t_" + func_name, False)
                d_idx = 2
                while d_idx < len(node.children):
                    if node.children[d_idx].type == "DECORATOR":
                        dec_reg = self.gen_expr(node.children[d_idx].children[0], False)
                        cur_fn = self.new_reg()
                        self.emit(cur_fn + " = load i64, i64* @t_" + func_name, False)
                        p_arg = self.emit_args_array([cur_fn], False)
                        new_fn = self.new_reg()
                        self.emit(new_fn + " = call i64 @turbo_call(i64 " + dec_reg + ", i32 1, i64* " + p_arg + ")", False)
                        self.emit("store i64 " + new_fn + ", i64* @t_" + func_name, False)
                    d_idx = d_idx + 1

    def gen_class_def(self, node, mod_prefix):
        class_name = node.value
        full_name = mod_prefix + class_name
        self.add_global(full_name)
        if mod_prefix == "":
            sp_cn = self.get_str_ptr(class_name, False)
            r_cls = self.new_reg()
            self.emit(r_cls + " = call i64 @make_class(i8* " + sp_cn + ")", False)
            self.emit("store i64 " + r_cls + ", i64* @t_" + class_name, False)

        suite = node.children[0]

        is_dataclass = False
        dc_idx = 1
        while dc_idx < len(node.children):
            if node.children[dc_idx].type == "DECORATOR":
                dcc = node.children[dc_idx].children[0]
                if dcc.type == "NAME" and dcc.value == "dataclass":
                    is_dataclass = True
            dc_idx = dc_idx + 1

        if is_dataclass:
            init_fields = []
            dc_m_idx = 0
            while dc_m_idx < len(suite.children):
                s = suite.children[dc_m_idx]
                if s.type == "ANN_ASSIGN":
                    fname = s.value
                    fdefault = s.children[0] if s.children[1].type == "CONST_NONE" else s.children[1]
                    init_fields.append([fname, fdefault])
                elif s.type == "ASSIGN":
                    target = s.children[0]
                    if target.type == "NAME":
                        fname = target.value
                        fdefault = s.children[1]
                        init_fields.append([fname, fdefault])
                dc_m_idx = dc_m_idx + 1
            init_impl = "t_impl_" + full_name + "__init__"
            saved_cur_lines = self.cur_lines
            saved_locals = self.local_vars
            saved_allocas = self.local_allocas
            saved_sized = self.sized_allocas
            saved_typed = self.typed_allocas
            saved_term = self.block_terminated
            self.cur_lines = []
            self.local_vars = ["self"]
            self.local_allocas = []
            self.sized_allocas = []
            self.typed_allocas = []
            self.block_terminated = False

            p_self = self.new_reg()
            self.emit(p_self + " = getelementptr inbounds i64, i64* %args, i64 0", True)
            v_self = self.new_reg()
            self.emit(v_self + " = load i64, i64* " + p_self, True)

            init_i = 0
            while init_i < len(init_fields):
                fname = init_fields[init_i][0]
                fdefault = init_fields[init_i][1]
                self.ensure_alloca(fname, True)
                c_arg = self.new_reg()
                self.emit(c_arg + " = icmp sgt i32 %argc, " + str(init_i + 1), True)
                lbl_has = self.new_label("dc_has")
                lbl_def = self.new_label("dc_def")
                lbl_done = self.new_label("dc_done")
                self.emit_cond_br(c_arg, lbl_has, lbl_def, True)
                self.emit_label(lbl_has, True)
                pa = self.new_reg()
                self.emit(pa + " = getelementptr inbounds i64, i64* %args, i64 " + str(init_i + 1), True)
                va = self.new_reg()
                self.emit(va + " = load i64, i64* " + pa, True)
                self.emit("store i64 " + va + ", i64* %loc_" + fname, True)
                self.emit_br(lbl_done, True)
                self.emit_label(lbl_def, True)
                if fdefault.type == "CONST_NONE":
                    self.emit("store i64 -1688849860263936, i64* %loc_" + fname, True)
                else:
                    vd = self.gen_expr(fdefault, True)
                    self.emit("store i64 " + vd + ", i64* %loc_" + fname, True)
                self.emit_br(lbl_done, True)
                self.emit_label(lbl_done, True)
                fv = self.new_reg()
                self.emit(fv + " = load i64, i64* %loc_" + fname, True)
                sp_f = self.get_str_ptr(fname, True)
                self.emit("call void @turbo_setattr(i64 " + v_self + ", i8* " + sp_f + ", i64 " + fv + ")", True)
                init_i = init_i + 1

            self.emit_ret("-1688849860263936", True)
            fn_lines = ["define i64 @" + init_impl + "(i32 %argc, i64* %args) {", "entry:"]
            a_i = 0
            while a_i < len(self.local_allocas):
                aname = self.local_allocas[a_i]
                fn_lines.append("  %loc_" + aname + " = alloca i64")
                fn_lines.append("  store i64 -1688849860263936, i64* %loc_" + aname)
                a_i = a_i + 1
            a_i = 0
            while a_i < len(self.typed_allocas):
                tinfo = self.typed_allocas[a_i]
                fn_lines.append("  %loc_" + tinfo[0] + " = alloca " + tinfo[1])
                a_i = a_i + 1
            a_i = 0
            while a_i < len(self.sized_allocas):
                sinfo = self.sized_allocas[a_i]
                fn_lines.append("  %loc_" + sinfo[0] + " = alloca [" + str(sinfo[1]) + " x i64]")
                a_i = a_i + 1
            c_i = 0
            while c_i < len(self.cur_lines):
                fn_lines.append(self.cur_lines[c_i])
                c_i = c_i + 1
            fn_lines.append("}")
            self.funcs_list.append("\n".join(fn_lines))

            self.cur_lines = saved_cur_lines
            self.local_vars = saved_locals
            self.local_allocas = saved_allocas
            self.sized_allocas = saved_sized
            self.typed_allocas = saved_typed
            self.block_terminated = saved_term

            if mod_prefix == "":
                r_c = self.new_reg()
                self.emit(r_c + " = load i64, i64* @t_" + class_name, False)
                sp_init = self.get_str_ptr("__init__", False)
                self.emit("call void @turbo_class_add_method(i64 " + r_c + ", i8* " + sp_init + ", i64 (i32, i64*)* @" + init_impl + ")", False)

        m_idx = 0
        while m_idx < len(suite.children):
            m_node = suite.children[m_idx]
            if m_node.type == "DEF" or m_node.type == "ASYNC_DEF":
                method_name = m_node.value
                is_static = False
                is_class = False
                is_property = False
                md_idx = 2
                while md_idx < len(m_node.children):
                    if m_node.children[md_idx].type == "DECORATOR":
                        dec_c = m_node.children[md_idx].children[0]
                        if dec_c.type == "NAME":
                            if dec_c.value == "staticmethod":
                                is_static = True
                            elif dec_c.value == "classmethod":
                                is_class = True
                            elif dec_c.value == "property":
                                is_property = True
                    md_idx = md_idx + 1
                self.gen_func_def(m_node, full_name, "", False)
                if mod_prefix == "":
                    r_c = self.new_reg()
                    self.emit(r_c + " = load i64, i64* @t_" + class_name, False)
                    sp_m = self.get_str_ptr(method_name, False)
                    m_impl = "@t_impl_" + class_name + "_" + method_name
                    if is_static:
                        sp_m2 = self.get_str_ptr(method_name, False)
                        r_sm = self.new_reg()
                        self.emit(r_sm + " = call i64 @make_staticmethod(i64 (i32, i64*)* " + m_impl + ", i8* " + sp_m2 + ")", False)
                        self.emit("call void @turbo_class_set_attr(i64 " + r_c + ", i8* " + sp_m + ", i64 " + r_sm + ")", False)
                    elif is_class:
                        sp_m2 = self.get_str_ptr(method_name, False)
                        r_cm = self.new_reg()
                        self.emit(r_cm + " = call i64 @make_classmethod(i64 (i32, i64*)* " + m_impl + ", i8* " + sp_m2 + ")", False)
                        self.emit("call void @turbo_class_set_attr(i64 " + r_c + ", i8* " + sp_m + ", i64 " + r_cm + ")", False)
                    elif is_property:
                        sp_m2 = self.get_str_ptr(method_name, False)
                        r_pm = self.new_reg()
                        self.emit(r_pm + " = call i64 @make_property(i64 (i32, i64*)* " + m_impl + ", i64 (i32, i64*)* null, i8* " + sp_m2 + ")", False)
                        self.emit("call void @turbo_class_set_attr(i64 " + r_c + ", i8* " + sp_m + ", i64 " + r_pm + ")", False)
                    else:
                        self.emit("call void @turbo_class_add_method(i64 " + r_c + ", i8* " + sp_m + ", i64 (i32, i64*)* " + m_impl + ")", False)
            elif m_node.type == "ASSIGN":
                target = m_node.children[0]
                if target.type == "NAME":
                    attr_name = target.value
                    if mod_prefix == "":
                        val_reg = self.gen_expr(m_node.children[1], False)
                        r_c = self.new_reg()
                        self.emit(r_c + " = load i64, i64* @t_" + class_name, False)
                        sp_a = self.get_str_ptr(attr_name, False)
                        self.emit("call void @turbo_class_set_attr(i64 " + r_c + ", i8* " + sp_a + ", i64 " + val_reg + ")", False)
            elif m_node.type == "ANN_ASSIGN":
                attr_name = m_node.value
                if len(m_node.children) > 1 and m_node.children[1].type != "CONST_NONE":
                    if mod_prefix == "":
                        val_reg = self.gen_expr(m_node.children[1], False)
                        r_c = self.new_reg()
                        self.emit(r_c + " = load i64, i64* @t_" + class_name, False)
                        sp_a = self.get_str_ptr(attr_name, False)
                        self.emit("call void @turbo_class_set_attr(i64 " + r_c + ", i8* " + sp_a + ", i64 " + val_reg + ")", False)
            m_idx = m_idx + 1

        d_idx = 1
        while d_idx < len(node.children):
            if node.children[d_idx].type == "DECORATOR":
                dec_c = node.children[d_idx].children[0]
                is_special = False
                if dec_c.type == "NAME" and dec_c.value == "dataclass":
                    is_special = True
                if not is_special and mod_prefix == "":
                    dec_reg = self.gen_expr(dec_c, False)
                    r_c = self.new_reg()
                    self.emit(r_c + " = load i64, i64* @t_" + class_name, False)
                    p_arg = self.emit_args_array([r_c], False)
                    r_newc = self.new_reg()
                    self.emit(r_newc + " = call i64 @turbo_call(i64 " + dec_reg + ", i32 1, i64* " + p_arg + ")", False)
                    self.emit("store i64 " + r_newc + ", i64* @t_" + class_name, False)
            d_idx = d_idx + 1

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
            return
        init_func_name = "t_init_module_" + module_name.replace('.', '_')
        if not (module_name in self.processed_modules):
            module_ast = self._load_module(module_name)
            if module_ast == None:
                return
            self.processed_modules.append(module_name)
            self.gen_module_init(module_name, module_ast)
        r_mod = self.new_reg()
        self.emit(r_mod + " = call i64 @" + init_func_name + "()", is_in_func)
        self.emit_store_var(local_name, r_mod, is_in_func)

    def _gen_from_import(self, node, is_in_func):
        module_name = node.value
        imported = node.children
        if is_stdlib_module(module_name.split('.')[0]):
            return
        if not (module_name in self.processed_modules):
            module_ast = self._load_module(module_name)
            if module_ast == None:
                return
            self.processed_modules.append(module_name)
            self.gen_module_init(module_name, module_ast)
        init_func_name = "t_init_module_" + module_name.replace('.', '_')
        i_idx = 0
        while i_idx < len(imported):
            item = imported[i_idx]
            if item.type == "NAME":
                orig_name = item.value
                r_mod = self.new_reg()
                self.emit(r_mod + " = call i64 @" + init_func_name + "()", is_in_func)
                sp_n = self.get_str_ptr(orig_name, is_in_func)
                r_val = self.new_reg()
                self.emit(r_val + " = call i64 @turbo_module_get(i64 " + r_mod + ", i8* " + sp_n + ")", is_in_func)
                self.emit_store_var(orig_name, r_val, is_in_func)
            elif item.type == "IMPORT_AS":
                orig_name = item.children[0].value
                alias_name = item.children[1].value
                r_mod = self.new_reg()
                self.emit(r_mod + " = call i64 @" + init_func_name + "()", is_in_func)
                sp_n = self.get_str_ptr(orig_name, is_in_func)
                r_val = self.new_reg()
                self.emit(r_val + " = call i64 @turbo_module_get(i64 " + r_mod + ", i8* " + sp_n + ")", is_in_func)
                self.emit_store_var(alias_name, r_val, is_in_func)
            i_idx = i_idx + 1

    def gen_module_init(self, module_name, module_ast):
        mod_slug = module_name.replace('.', '_')
        prefix = mod_slug + '_'
        init_func_name = "t_init_module_" + mod_slug

        mod_globals = []
        collect_globals(module_ast, mod_globals)
        g_idx = 0
        while g_idx < len(mod_globals):
            g_name = mod_globals[g_idx]
            if not self.is_builtin_name(g_name):
                self.add_global(prefix + g_name)
            g_idx = g_idx + 1

        saved_mp = self.mod_prefix
        saved_mlg = self.mod_local_globals
        self.mod_prefix = prefix
        self.mod_local_globals = mod_globals

        stmts = module_ast.children
        s_idx = 0
        while s_idx < len(stmts):
            stmt = stmts[s_idx]
            if stmt.type == "DEF" or stmt.type == "ASYNC_DEF":
                old_is_gen = self.is_generator
                self.gen_func_def(stmt, "", prefix, False)
                self.is_generator = old_is_gen
            elif stmt.type == "CLASS":
                old_is_gen = self.is_generator
                self.gen_class_def(stmt, prefix)
                self.is_generator = old_is_gen
            s_idx = s_idx + 1

        done_g = "@" + init_func_name + "_done"
        obj_g = "@" + init_func_name + "_obj"
        self.raw_globals.append(done_g + " = global i32 0")
        self.raw_globals.append(obj_g + " = global i64 -1688849860263936")

        saved_cur_lines = self.cur_lines
        saved_locals = self.local_vars
        saved_allocas = self.local_allocas
        saved_sized = self.sized_allocas
        saved_typed = self.typed_allocas
        saved_term = self.block_terminated

        self.cur_lines = []
        self.local_vars = []
        self.local_allocas = []
        self.sized_allocas = []
        self.typed_allocas = []
        self.block_terminated = False

        r_done = self.new_reg()
        self.emit(r_done + " = load i32, i32* " + done_g, True)
        r_is_done = self.new_reg()
        self.emit(r_is_done + " = icmp ne i32 " + r_done + ", 0", True)
        lbl_cached = self.new_label("mod_cached")
        lbl_init = self.new_label("mod_init")
        self.emit_cond_br(r_is_done, lbl_cached, lbl_init, True)

        self.emit_label(lbl_cached, True)
        r_cmod = self.new_reg()
        self.emit(r_cmod + " = load i64, i64* " + obj_g, True)
        self.emit_ret(r_cmod, True)

        self.emit_label(lbl_init, True)
        self.emit("store i32 1, i32* " + done_g, True)
        r_mod = self.new_reg()
        self.emit(r_mod + " = call i64 @make_module()", True)

        s_idx = 0
        while s_idx < len(stmts):
            stmt = stmts[s_idx]
            if stmt.type == "DEF" or stmt.type == "ASYNC_DEF":
                func_name = stmt.value
                sp_f = self.get_str_ptr(func_name, True)
                r_fn = self.new_reg()
                self.emit(r_fn + " = call i64 @make_func(i64 (i32, i64*)* @t_impl_" + prefix + func_name + ", i8* " + sp_f + ")", True)
                self.emit("store i64 " + r_fn + ", i64* @t_" + prefix + func_name, True)
                sp_f2 = self.get_str_ptr(func_name, True)
                self.emit("call void @turbo_module_set(i64 " + r_mod + ", i8* " + sp_f2 + ", i64 " + r_fn + ")", True)
            elif stmt.type == "CLASS":
                class_name = stmt.value
                sp_c = self.get_str_ptr(class_name, True)
                r_cls = self.new_reg()
                self.emit(r_cls + " = call i64 @make_class(i8* " + sp_c + ")", True)
                self.emit("store i64 " + r_cls + ", i64* @t_" + prefix + class_name, True)
                suite = stmt.children[0]
                m_idx = 0
                while m_idx < len(suite.children):
                    m_node = suite.children[m_idx]
                    if m_node.type == "DEF" or m_node.type == "ASYNC_DEF":
                        method_name = m_node.value
                        sp_m = self.get_str_ptr(method_name, True)
                        self.emit("call void @turbo_class_add_method(i64 " + r_cls + ", i8* " + sp_m + ", i64 (i32, i64*)* @t_impl_" + prefix + class_name + "_" + method_name + ")", True)
                    m_idx = m_idx + 1
                sp_c2 = self.get_str_ptr(class_name, True)
                self.emit("call void @turbo_module_set(i64 " + r_mod + ", i8* " + sp_c2 + ", i64 " + r_cls + ")", True)
            elif stmt.type == "ASSIGN":
                target = stmt.children[0]
                val_node = stmt.children[1]
                if target.type == "NAME":
                    var_name = target.value
                    val_reg = self.gen_expr(val_node, True)
                    self.emit("store i64 " + val_reg + ", i64* @t_" + prefix + var_name, True)
                    sp_v = self.get_str_ptr(var_name, True)
                    self.emit("call void @turbo_module_set(i64 " + r_mod + ", i8* " + sp_v + ", i64 " + val_reg + ")", True)
            elif stmt.type == "AUGASSIGN":
                target = stmt.children[0]
                val_node = stmt.children[1]
                op = stmt.value
                if target.type == "NAME" and op == '+=':
                    var_name = target.value
                    v_old = self.new_reg()
                    self.emit(v_old + " = load i64, i64* @t_" + prefix + var_name, True)
                    v_rhs = self.gen_expr(val_node, True)
                    v_new = self.new_reg()
                    self.emit(v_new + " = call i64 @turbo_add(i64 " + v_old + ", i64 " + v_rhs + ")", True)
                    self.emit("store i64 " + v_new + ", i64* @t_" + prefix + var_name, True)
                    sp_v = self.get_str_ptr(var_name, True)
                    self.emit("call void @turbo_module_set(i64 " + r_mod + ", i8* " + sp_v + ", i64 " + v_new + ")", True)
            elif stmt.type == "EXPR":
                self.gen_expr(stmt.children[0], True)
            s_idx = s_idx + 1

        self.emit("store i64 " + r_mod + ", i64* " + obj_g, True)
        self.emit_ret(r_mod, True)

        fn_lines = ["define i64 @" + init_func_name + "() {", "entry:"]
        a_i = 0
        while a_i < len(self.local_allocas):
            aname = self.local_allocas[a_i]
            fn_lines.append("  %loc_" + aname + " = alloca i64")
            fn_lines.append("  store i64 -1688849860263936, i64* %loc_" + aname)
            a_i = a_i + 1
        a_i = 0
        while a_i < len(self.typed_allocas):
            tinfo = self.typed_allocas[a_i]
            fn_lines.append("  %loc_" + tinfo[0] + " = alloca " + tinfo[1])
            a_i = a_i + 1
        a_i = 0
        while a_i < len(self.sized_allocas):
            sinfo = self.sized_allocas[a_i]
            fn_lines.append("  %loc_" + sinfo[0] + " = alloca [" + str(sinfo[1]) + " x i64]")
            a_i = a_i + 1
        c_i = 0
        while c_i < len(self.cur_lines):
            fn_lines.append(self.cur_lines[c_i])
            c_i = c_i + 1
        fn_lines.append("}")
        self.funcs_list.append("\n".join(fn_lines))

        self.cur_lines = saved_cur_lines
        self.local_vars = saved_locals
        self.local_allocas = saved_allocas
        self.sized_allocas = saved_sized
        self.typed_allocas = saved_typed
        self.block_terminated = saved_term
        self.mod_prefix = saved_mp
        self.mod_local_globals = saved_mlg

    NATIVE_FLOAT = "float"
    NATIVE_INT = "int"

    def _infer_native_type(self, node, var_types, loop_var):
        if node.type == "CONST_FLOAT":
            return CodeGen.NATIVE_FLOAT
        if node.type == "CONST_INT":
            return CodeGen.NATIVE_INT
        if node.type == "NAME":
            if node.value == loop_var:
                return CodeGen.NATIVE_INT
            return var_types.get(node.value, None)
        if node.type == "UNARY" and node.value == "-":
            return self._infer_native_type(node.children[0], var_types, loop_var)
        if node.type == "BINOP":
            lt = self._infer_native_type(node.children[0], var_types, loop_var)
            rt = self._infer_native_type(node.children[1], var_types, loop_var)
            if lt == None or rt == None:
                return None
            if lt == CodeGen.NATIVE_FLOAT or rt == CodeGen.NATIVE_FLOAT:
                if node.value in ('+', '-', '*', '/'):
                    return CodeGen.NATIVE_FLOAT
                return None
            if lt == CodeGen.NATIVE_INT and rt == CodeGen.NATIVE_INT:
                if node.value in ('+', '-', '*', '//', '%'):
                    return CodeGen.NATIVE_INT
                if node.value == '/':
                    return CodeGen.NATIVE_FLOAT
                return None
            return None
        return None

    def _can_emit_native_expr(self, node, var_types, loop_var):
        if node.type == "CONST_INT":
            return CodeGen.NATIVE_INT
        if node.type == "CONST_FLOAT":
            return CodeGen.NATIVE_FLOAT
        if node.type == "NAME":
            if node.value == loop_var:
                return CodeGen.NATIVE_INT
            return var_types.get(node.value, None)
        if node.type == "UNARY" and node.value == "-":
            return self._can_emit_native_expr(node.children[0], var_types, loop_var)
        if node.type == "BINOP":
            lt = self._can_emit_native_expr(node.children[0], var_types, loop_var)
            rt = self._can_emit_native_expr(node.children[1], var_types, loop_var)
            if lt == None or rt == None:
                return None
            op = node.value
            if lt == CodeGen.NATIVE_FLOAT or rt == CodeGen.NATIVE_FLOAT:
                if op in ('+', '-', '*', '/'):
                    return CodeGen.NATIVE_FLOAT
                return None
            if lt == CodeGen.NATIVE_INT and rt == CodeGen.NATIVE_INT:
                if op == '/':
                    return CodeGen.NATIVE_FLOAT
                if op in ('+', '-', '*', '//', '%'):
                    return CodeGen.NATIVE_INT
                return None
        return None

    def _can_emit_native_stmt(self, stmt, var_types, loop_var):
        if stmt.type == "ASSIGN":
            target = stmt.children[0]
            if target.type == "NAME":
                et = self._can_emit_native_expr(stmt.children[1], var_types, loop_var)
                if et == None:
                    return False
                return var_types.get(target.value, None) != None
            elif target.type == "TUPLE":
                val = stmt.children[1]
                if val.type == "TUPLE":
                    if len(target.children) != len(val.children):
                        return False
                    i = 0
                    while i < len(target.children):
                        if target.children[i].type != "NAME":
                            return False
                        et = self._can_emit_native_expr(val.children[i], var_types, loop_var)
                        if et == None:
                            return False
                        if var_types.get(target.children[i].value, None) == None:
                            return False
                        i = i + 1
                    return True
            return False
        if stmt.type == "AUGASSIGN":
            target = stmt.children[0]
            if target.type == "NAME":
                if not (stmt.value in ('+=', '-=', '*=', '/=')):
                    return False
                et = self._can_emit_native_expr(stmt.children[1], var_types, loop_var)
                if et == None:
                    return False
                return var_types.get(target.value, None) != None
        return False

    def _emit_native_expr(self, node, var_types, loop_var, is_in_func):
        if node.type == "CONST_INT":
            return [str(int(node.value)), CodeGen.NATIVE_INT]
        if node.type == "CONST_FLOAT":
            return [format_llvm_float(node.value), CodeGen.NATIVE_FLOAT]
        if node.type == "NAME":
            if node.value == loop_var:
                r = self.new_reg()
                self.emit(r + " = load i64, i64* %loc__r_i", is_in_func)
                return [r, CodeGen.NATIVE_INT]
            t = var_types.get(node.value, None)
            if t == CodeGen.NATIVE_FLOAT:
                r = self.new_reg()
                self.emit(r + " = load double, double* %loc__d_" + node.value, is_in_func)
                return [r, CodeGen.NATIVE_FLOAT]
            if t == CodeGen.NATIVE_INT:
                r = self.new_reg()
                self.emit(r + " = load i64, i64* %loc__i_" + node.value, is_in_func)
                return [r, CodeGen.NATIVE_INT]
            return None
        if node.type == "UNARY" and node.value == "-":
            inner = self._emit_native_expr(node.children[0], var_types, loop_var, is_in_func)
            if inner == None:
                return None
            r = self.new_reg()
            if inner[1] == CodeGen.NATIVE_FLOAT:
                self.emit(r + " = fneg double " + inner[0], is_in_func)
                return [r, CodeGen.NATIVE_FLOAT]
            else:
                self.emit(r + " = sub i64 0, " + inner[0], is_in_func)
                return [r, CodeGen.NATIVE_INT]
        if node.type == "BINOP":
            l_res = self._emit_native_expr(node.children[0], var_types, loop_var, is_in_func)
            if l_res == None:
                return None
            r_res = self._emit_native_expr(node.children[1], var_types, loop_var, is_in_func)
            if r_res == None:
                return None
            lv = l_res[0]
            lt = l_res[1]
            rv = r_res[0]
            rt = r_res[1]
            op = node.value
            if lt == CodeGen.NATIVE_FLOAT or rt == CodeGen.NATIVE_FLOAT or op == '/':
                if lt == CodeGen.NATIVE_INT:
                    r_conv = self.new_reg()
                    self.emit(r_conv + " = sitofp i64 " + lv + " to double", is_in_func)
                    lv = r_conv
                if rt == CodeGen.NATIVE_INT:
                    r_conv = self.new_reg()
                    self.emit(r_conv + " = sitofp i64 " + rv + " to double", is_in_func)
                    rv = r_conv
                f_op = "fadd"
                if op == '-':
                    f_op = "fsub"
                elif op == '*':
                    f_op = "fmul"
                elif op == '/':
                    f_op = "fdiv"
                r = self.new_reg()
                self.emit(r + " = " + f_op + " double " + lv + ", " + rv, is_in_func)
                return [r, CodeGen.NATIVE_FLOAT]
            if lt == CodeGen.NATIVE_INT and rt == CodeGen.NATIVE_INT:
                i_op = "add"
                if op == '-':
                    i_op = "sub"
                elif op == '*':
                    i_op = "mul"
                elif op == '//':
                    i_op = "sdiv"
                elif op == '%':
                    i_op = "srem"
                r = self.new_reg()
                self.emit(r + " = " + i_op + " i64 " + lv + ", " + rv, is_in_func)
                return [r, CodeGen.NATIVE_INT]
        return None

    def _emit_native_stmt(self, stmt, var_types, loop_var, is_in_func):
        if stmt.type == "ASSIGN":
            target = stmt.children[0]
            if target.type == "NAME":
                res = self._emit_native_expr(stmt.children[1], var_types, loop_var, is_in_func)
                if res == None:
                    return False
                val_r = res[0]
                vt = res[1]
                t = var_types.get(target.value, None)
                if t == CodeGen.NATIVE_FLOAT:
                    if vt == CodeGen.NATIVE_INT:
                        rc = self.new_reg()
                        self.emit(rc + " = sitofp i64 " + val_r + " to double", is_in_func)
                        val_r = rc
                    self.emit("store double " + val_r + ", double* %loc__d_" + target.value, is_in_func)
                    return True
                if t == CodeGen.NATIVE_INT:
                    if vt == CodeGen.NATIVE_FLOAT:
                        rc = self.new_reg()
                        self.emit(rc + " = fptosi double " + val_r + " to i64", is_in_func)
                        val_r = rc
                    self.emit("store i64 " + val_r + ", i64* %loc__i_" + target.value, is_in_func)
                    return True
            elif target.type == "TUPLE":
                val = stmt.children[1]
                if val.type == "TUPLE":
                    eval_regs = []
                    i = 0
                    while i < len(target.children):
                        res = self._emit_native_expr(val.children[i], var_types, loop_var, is_in_func)
                        if res == None:
                            return False
                        vr = res[0]
                        vt = res[1]
                        vname = target.children[i].value
                        lhs_t = var_types.get(vname, None)
                        if lhs_t == CodeGen.NATIVE_FLOAT and vt == CodeGen.NATIVE_INT:
                            rc = self.new_reg()
                            self.emit(rc + " = sitofp i64 " + vr + " to double", is_in_func)
                            vr = rc
                        elif lhs_t == CodeGen.NATIVE_INT and vt == CodeGen.NATIVE_FLOAT:
                            rc = self.new_reg()
                            self.emit(rc + " = fptosi double " + vr + " to i64", is_in_func)
                            vr = rc
                        eval_regs.append(vr)
                        i = i + 1
                    i = 0
                    while i < len(target.children):
                        vname = target.children[i].value
                        lhs_t = var_types.get(vname, None)
                        if lhs_t == CodeGen.NATIVE_FLOAT:
                            self.emit("store double " + eval_regs[i] + ", double* %loc__d_" + vname, is_in_func)
                        elif lhs_t == CodeGen.NATIVE_INT:
                            self.emit("store i64 " + eval_regs[i] + ", i64* %loc__i_" + vname, is_in_func)
                        i = i + 1
                    return True
        if stmt.type == "AUGASSIGN":
            target = stmt.children[0]
            if target.type == "NAME":
                res = self._emit_native_expr(stmt.children[1], var_types, loop_var, is_in_func)
                if res == None:
                    return False
                val_r = res[0]
                vt = res[1]
                t = var_types.get(target.value, None)
                op = stmt.value[0]
                if t == CodeGen.NATIVE_FLOAT:
                    if vt == CodeGen.NATIVE_INT:
                        rc = self.new_reg()
                        self.emit(rc + " = sitofp i64 " + val_r + " to double", is_in_func)
                        val_r = rc
                    cur_r = self.new_reg()
                    self.emit(cur_r + " = load double, double* %loc__d_" + target.value, is_in_func)
                    f_op = "fadd"
                    if op == '-':
                        f_op = "fsub"
                    elif op == '*':
                        f_op = "fmul"
                    elif op == '/':
                        f_op = "fdiv"
                    new_r = self.new_reg()
                    self.emit(new_r + " = " + f_op + " double " + cur_r + ", " + val_r, is_in_func)
                    self.emit("store double " + new_r + ", double* %loc__d_" + target.value, is_in_func)
                    return True
                if t == CodeGen.NATIVE_INT:
                    cur_r = self.new_reg()
                    self.emit(cur_r + " = load i64, i64* %loc__i_" + target.value, is_in_func)
                    i_op = "add"
                    if op == '-':
                        i_op = "sub"
                    elif op == '*':
                        i_op = "mul"
                    new_r = self.new_reg()
                    self.emit(new_r + " = " + i_op + " i64 " + cur_r + ", " + val_r, is_in_func)
                    self.emit("store i64 " + new_r + ", i64* %loc__i_" + target.value, is_in_func)
                    return True
        return False

    def _collect_names(self, node, names):
        if node.type == "NAME":
            names.add(node.value)
        if hasattr(node, 'children') and node.children:
            for c in node.children:
                if isinstance(c, ASTNode):
                    self._collect_names(c, names)

    def _infer_type_heuristic(self, node, var_types, loop_var):
        if node.type == "NAME":
            if node.value == loop_var:
                return CodeGen.NATIVE_INT
            if node.value in var_types:
                return var_types[node.value]
            return None
        if node.type == "CONST_FLOAT":
            return CodeGen.NATIVE_FLOAT
        if node.type == "CONST_INT":
            return CodeGen.NATIVE_INT
        if node.type == "UNARY" and node.value == "-":
            return self._infer_type_heuristic(node.children[0], var_types, loop_var)
        if node.type == "BINOP":
            lt = self._infer_type_heuristic(node.children[0], var_types, loop_var)
            rt = self._infer_type_heuristic(node.children[1], var_types, loop_var)
            op = node.value
            if op == '/':
                return CodeGen.NATIVE_FLOAT
            if lt == CodeGen.NATIVE_FLOAT or rt == CodeGen.NATIVE_FLOAT:
                if op in ('+', '-', '*'):
                    return CodeGen.NATIVE_FLOAT
                return None
            if lt == CodeGen.NATIVE_INT and rt == CodeGen.NATIVE_INT:
                if op in ('+', '-', '*', '//', '%'):
                    return CodeGen.NATIVE_INT
                return None
            if lt != None:
                return lt
            if rt != None:
                return rt
            return None
        return None

    def _infer_func_types(self, body_node, var_types):
        for stmt in body_node.children:
            if stmt.type == "ASSIGN":
                target = stmt.children[0]
                if target.type == "NAME":
                    val = stmt.children[1]
                    t = self._infer_type_heuristic(val, var_types, None)
                    if t != None:
                        var_types[target.value] = t
                elif target.type == "TUPLE":
                    val = stmt.children[1]
                    if val.type == "TUPLE":
                        for i in range(len(target.children)):
                            if target.children[i].type == "NAME" and i < len(val.children):
                                t = self._infer_type_heuristic(val.children[i], var_types, None)
                                if t != None:
                                    var_types[target.children[i].value] = t
            elif stmt.type == "AUGASSIGN":
                target = stmt.children[0]
                if target.type == "NAME" and target.value in var_types:
                    pass
            elif stmt.type == "FOR":
                iter_node = stmt.children[0]
                if iter_node.type == "CALL" and iter_node.children[0].type == "NAME" and iter_node.children[0].value == "range":
                    var_types[stmt.value] = CodeGen.NATIVE_INT
                body = stmt.children[1]
                self._infer_func_types(body, var_types)
            elif stmt.type == "WHILE":
                body = stmt.children[1]
                self._infer_func_types(body, var_types)
            elif stmt.type == "IF":
                _i = 0
                while _i < len(stmt.children):
                    if _i + 1 < len(stmt.children):
                        body = stmt.children[_i + 1]
                        self._infer_func_types(body, var_types)
                    else:
                        self._infer_func_types(stmt.children[_i], var_types)
                    _i = _i + 2
            elif stmt.type == "TYPEDEF" or stmt.type == "DEF" or stmt.type == "ASYNC_DEF":
                pass

    def _type_from_body_use(self, var_name, body_node, loop_var):
        var_types_dummy = {loop_var: CodeGen.NATIVE_INT}
        for stmt in body_node.children:
            if stmt.type == "ASSIGN":
                target = stmt.children[0]
                val = stmt.children[1]
                if target.type == "NAME" and target.value == var_name:
                    t = self._infer_type_heuristic(val, var_types_dummy, loop_var)
                    if t != None:
                        return t
                elif target.type == "TUPLE" and val.type == "TUPLE":
                    for i in range(len(target.children)):
                        if target.children[i].type == "NAME" and target.children[i].value == var_name and i < len(val.children):
                            t = self._infer_type_heuristic(val.children[i], var_types_dummy, loop_var)
                            if t != None:
                                return t
            elif stmt.type == "AUGASSIGN":
                val = stmt.children[1]
                t = self._infer_type_heuristic(val, var_types_dummy, loop_var)
                if t != None:
                    return t
        return None

    def _collect_op_usage(self, body_node):
        ops = {}
        stmt_idx = 0
        while stmt_idx < len(body_node.children):
            stmt = body_node.children[stmt_idx]
            nodes = [stmt]
            while len(nodes) > 0:
                node = nodes[0]
                nodes = nodes[1:]
                if node.type == "BINOP":
                    op = node.value
                    c_idx = 0
                    while c_idx < len(node.children):
                        c = node.children[c_idx]
                        if c.type == "NAME":
                            existing = ops.get(c.value, None)
                            if existing == None:
                                existing = []
                            ops[c.value] = existing + [op]
                        if isinstance(c, ASTNode):
                            nodes.append(c)
                        c_idx = c_idx + 1
                elif node.type == "UNARY":
                    c_idx = 0
                    while c_idx < len(node.children):
                        c = node.children[c_idx]
                        if isinstance(c, ASTNode):
                            nodes.append(c)
                        c_idx = c_idx + 1
            stmt_idx = stmt_idx + 1
        return ops

    def _scan_body_vars(self, body_node, var_types, loop_var):
        lhs_map = {}
        lhs_keys_list = []
        stmt_types = []
        s_idx = 0
        while s_idx < len(body_node.children):
            stmt = body_node.children[s_idx]
            stmt_types.append(stmt.type)
            if stmt.type == "ASSIGN" or stmt.type == "AUGASSIGN":
                target = stmt.children[0]
                if target.type == "NAME":
                    lhs_map[target.value] = stmt
                    lhs_keys_list.append(target.value)
                elif target.type == "TUPLE":
                    all_names = True
                    for elem in target.children:
                        if elem.type != "NAME":
                            all_names = False
                            break
                    if not all_names:
                        return False
                    for elem in target.children:
                        lhs_map[elem.value] = stmt
                        lhs_keys_list.append(elem.value)
                else:
                    return False
            s_idx = s_idx + 1
        t_idx = 0
        while t_idx < len(stmt_types):
            if stmt_types[t_idx] != "ASSIGN" and stmt_types[t_idx] != "AUGASSIGN":
                return False
            t_idx = t_idx + 1
        op_usage = self._collect_op_usage(body_node)
        changed = True
        while changed:
            changed = False
            k_idx = 0
            while k_idx < len(lhs_keys_list):
                v = lhs_keys_list[k_idx]
                if v in var_types:
                    k_idx = k_idx + 1
                    continue
                stmt = lhs_map[v]
                val = stmt.children[1]
                typ = self._infer_type_heuristic(val, var_types, loop_var)
                if typ != None:
                    var_types[v] = typ
                    changed = True
                k_idx = k_idx + 1
        k_idx = 0
        while k_idx < len(lhs_keys_list):
            v = lhs_keys_list[k_idx]
            if v in var_types:
                k_idx = k_idx + 1
                continue
            uses = op_usage.get(v, [])
            has_div = False
            u_idx = 0
            while u_idx < len(uses):
                if uses[u_idx] == "/":
                    has_div = True
                u_idx = u_idx + 1
            if has_div:
                var_types[v] = CodeGen.NATIVE_FLOAT
                changed = True
            else:
                has_arith = False
                u_idx = 0
                while u_idx < len(uses):
                    if uses[u_idx] == "+" or uses[u_idx] == "-" or uses[u_idx] == "*":
                        has_arith = True
                    u_idx = u_idx + 1
                if has_arith:
                    var_types[v] = CodeGen.NATIVE_FLOAT
                    changed = True
                else:
                    typ = self._type_from_body_use(v, body_node, loop_var)
                    if typ != None:
                        var_types[v] = typ
                        changed = True
                    else:
                        return False
            k_idx = k_idx + 1
        changed = True
        while changed:
            changed = False
            k_idx = 0
            while k_idx < len(lhs_keys_list):
                v = lhs_keys_list[k_idx]
                if not (v in var_types):
                    k_idx = k_idx + 1
                    continue
                stmt = lhs_map[v]
                val = stmt.children[1]
                t = self._infer_type_heuristic(val, var_types, loop_var)
                if t != None and t != var_types[v]:
                    var_types[v] = t
                    changed = True
                k_idx = k_idx + 1
        k_idx = 0
        while k_idx < len(lhs_keys_list):
            v = lhs_keys_list[k_idx]
            if not (v in var_types):
                return False
            k_idx = k_idx + 1
        # Verify every statement in body_node can be emitted natively
        s_idx = 0
        while s_idx < len(body_node.children):
            if not self._can_emit_native_stmt(body_node.children[s_idx], var_types, loop_var):
                return False
            s_idx = s_idx + 1
        return lhs_keys_list

    def gen_block_stmts(self, block_node, is_in_func):
        stmts = block_node.children
        s_idx = 0
        while s_idx < len(stmts):
            self.gen_stmt(stmts[s_idx], is_in_func)
            s_idx = s_idx + 1
        self._flush_pending_native(is_in_func)

    def _flush_pending_native(self, is_in_func):
        if self.pending_native_vars != None:
            pnv = self.pending_native_vars
            native_vars = pnv[0]
            native_int_vars = pnv[1]
            self.pending_native_vars = None
            for v in native_vars:
                rd = self.new_reg()
                self.emit(rd + " = load double, double* %loc__d_" + v, is_in_func)
                rb = self.emit_box_float(rd, is_in_func)
                self.emit_store_var(v, rb, is_in_func)
            for v in native_int_vars:
                ri = self.new_reg()
                self.emit(ri + " = load i64, i64* %loc__i_" + v, is_in_func)
                rb = self.emit_box_int(ri, is_in_func)
                self.emit_store_var(v, rb, is_in_func)

    def gen_stmt(self, node, is_in_func):
        if node.type == "PASS":
            return
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
            self._flush_pending_native(is_in_func)
            if len(self.loop_stack) > 0:
                top_loop = self.loop_stack[len(self.loop_stack) - 1]
                brk_lbl = top_loop[0]
                flag_var = top_loop[2]
                if flag_var != "":
                    self.emit("store i32 1, i32* %loc_" + flag_var, is_in_func)
                self.emit_br(brk_lbl, is_in_func)
        elif node.type == "CONTINUE":
            self._flush_pending_native(is_in_func)
            if len(self.loop_stack) > 0:
                top_loop = self.loop_stack[len(self.loop_stack) - 1]
                cont_lbl = top_loop[1]
                self.emit_br(cont_lbl, is_in_func)
        elif node.type == "ANN_ASSIGN":
            self._flush_pending_native(is_in_func)
            target_name = node.value
            default_node = node.children[1]
            if default_node.type != "CONST_NONE":
                val_reg = self.gen_expr(default_node, is_in_func)
                self.emit_store_var(target_name, val_reg, is_in_func)
        elif node.type == "EXPR":
            self._flush_pending_native(is_in_func)
            self.gen_expr(node.children[0], is_in_func)
        elif node.type == "ASSIGN":
            self._flush_pending_native(is_in_func)
            target = node.children[0]
            val_node = node.children[1]
            is_sys_argv_setup = False
            if target.type == "NAME" and target.value == "sys_argv":
                if val_node.type == "ATTR" and val_node.value == "argv":
                    if val_node.children[0].type == "NAME" and val_node.children[0].value == "sys":
                        is_sys_argv_setup = True
            if is_sys_argv_setup:
                return
            val_reg = self.gen_expr(val_node, is_in_func)
            if target.type == "NAME":
                self.emit_store_var(target.value, val_reg, is_in_func)
            elif target.type == "ATTR":
                obj_reg = self.gen_expr(target.children[0], is_in_func)
                sp_a = self.get_str_ptr(target.value, is_in_func)
                self.emit("call void @turbo_setattr(i64 " + obj_reg + ", i8* " + sp_a + ", i64 " + val_reg + ")", is_in_func)
            elif target.type == "SUBSCRIPT":
                obj_reg = self.gen_expr(target.children[0], is_in_func)
                idx_node = target.children[1]
                if idx_node.type == "NAME" and idx_node.value in self.range_loop_vars:
                    ri_loc = self.range_loop_vars[idx_node.value]
                    r_idx_i64 = self.new_reg()
                    self.emit(r_idx_i64 + " = load i64, i64* %loc_" + ri_loc, is_in_func)
                    self._emit_fast_list_setitem(obj_reg, r_idx_i64, val_reg, is_in_func)
                else:
                    idx_reg = self.gen_expr(idx_node, is_in_func)
                    self.emit("call void @turbo_setitem(i64 " + obj_reg + ", i64 " + idx_reg + ", i64 " + val_reg + ")", is_in_func)
            elif target.type == "TUPLE":
                r_len_tv = self.new_reg()
                self.emit(r_len_tv + " = call i64 @turbo_len(i64 " + val_reg + ")", is_in_func)
                r_len = self.emit_unbox_int(r_len_tv, is_in_func)
                e_idx = 0
                while e_idx < len(target.children):
                    elem = target.children[e_idx]
                    idx_const = str(-2251799813685248 + e_idx)
                    if elem.type == "NAME":
                        self.emit_store_var(elem.value, "-1688849860263936", is_in_func)
                        c_has = self.new_reg()
                        self.emit(c_has + " = icmp sgt i64 " + r_len + ", " + str(e_idx), is_in_func)
                        lbl_has = self.new_label("tup_has")
                        lbl_done = self.new_label("tup_done")
                        self.emit_cond_br(c_has, lbl_has, lbl_done, is_in_func)
                        self.emit_label(lbl_has, is_in_func)
                        r_it = self.new_reg()
                        self.emit(r_it + " = call i64 @turbo_getitem(i64 " + val_reg + ", i64 " + idx_const + ")", is_in_func)
                        self.emit_store_var(elem.value, r_it, is_in_func)
                        self.emit_br(lbl_done, is_in_func)
                        self.emit_label(lbl_done, is_in_func)
                    elif elem.type == "ATTR":
                        c_has = self.new_reg()
                        self.emit(c_has + " = icmp sgt i64 " + r_len + ", " + str(e_idx), is_in_func)
                        lbl_has = self.new_label("tup_has")
                        lbl_done = self.new_label("tup_done")
                        self.emit_cond_br(c_has, lbl_has, lbl_done, is_in_func)
                        self.emit_label(lbl_has, is_in_func)
                        obj_reg = self.gen_expr(elem.children[0], is_in_func)
                        r_it = self.new_reg()
                        self.emit(r_it + " = call i64 @turbo_getitem(i64 " + val_reg + ", i64 " + idx_const + ")", is_in_func)
                        sp_a = self.get_str_ptr(elem.value, is_in_func)
                        self.emit("call void @turbo_setattr(i64 " + obj_reg + ", i8* " + sp_a + ", i64 " + r_it + ")", is_in_func)
                        self.emit_br(lbl_done, is_in_func)
                        self.emit_label(lbl_done, is_in_func)
                    elif elem.type == "SUBSCRIPT":
                        c_has = self.new_reg()
                        self.emit(c_has + " = icmp sgt i64 " + r_len + ", " + str(e_idx), is_in_func)
                        lbl_has = self.new_label("tup_has")
                        lbl_done = self.new_label("tup_done")
                        self.emit_cond_br(c_has, lbl_has, lbl_done, is_in_func)
                        self.emit_label(lbl_has, is_in_func)
                        obj_reg = self.gen_expr(elem.children[0], is_in_func)
                        idx2_reg = self.gen_expr(elem.children[1], is_in_func)
                        r_it = self.new_reg()
                        self.emit(r_it + " = call i64 @turbo_getitem(i64 " + val_reg + ", i64 " + idx_const + ")", is_in_func)
                        self.emit("call void @turbo_setitem(i64 " + obj_reg + ", i64 " + idx2_reg + ", i64 " + r_it + ")", is_in_func)
                        self.emit_br(lbl_done, is_in_func)
                        self.emit_label(lbl_done, is_in_func)
                    elif elem.type == "TUPLE":
                        r_sub = self.new_reg()
                        self.emit(r_sub + " = call i64 @turbo_getitem(i64 " + val_reg + ", i64 " + idx_const + ")", is_in_func)
                        sub_idx = 0
                        while sub_idx < len(elem.children):
                            sub_elem = elem.children[sub_idx]
                            sub_idx_const = str(-2251799813685248 + sub_idx)
                            if sub_elem.type == "NAME":
                                r_sub_it = self.new_reg()
                                self.emit(r_sub_it + " = call i64 @turbo_getitem(i64 " + r_sub + ", i64 " + sub_idx_const + ")", is_in_func)
                                self.emit_store_var(sub_elem.value, r_sub_it, is_in_func)
                            sub_idx = sub_idx + 1
                    e_idx = e_idx + 1
        elif node.type == "AUGASSIGN":
            if self.pending_native_vars != None:
                pnv = self.pending_native_vars
                var_types = pnv[2]
                loop_var = pnv[3]
                if self._can_emit_native_stmt(node, var_types, loop_var):
                    self._emit_native_stmt(node, var_types, loop_var, is_in_func)
                    return
            self._flush_pending_native(is_in_func)
            target = node.children[0]
            val_reg = self.gen_expr(node.children[1], is_in_func)
            op = node.value
            if target.type == "NAME":
                old_reg = self.emit_load_var(target.value, is_in_func)
                fn = "@turbo_add"
                if op == '+=':
                    fn = "@turbo_add"
                elif op == '-=':
                    fn = "@turbo_sub"
                elif op == '*=':
                    fn = "@turbo_mul"
                elif op == '/=':
                    fn = "@turbo_div"
                elif op == '%=':
                    fn = "@turbo_mod"
                elif op == '**=':
                    fn = "@turbo_pow"
                elif op == '//=':
                    fn = "@turbo_floordiv"
                elif op == '&=':
                    fn = "@turbo_bitand"
                elif op == '|=':
                    fn = "@turbo_bitor"
                elif op == '^=':
                    fn = "@turbo_bitxor"
                elif op == '>>=':
                    fn = "@turbo_rshift"
                elif op == '<<=':
                    fn = "@turbo_lshift"
                elif op == '@=':
                    fn = "@turbo_matmul"
                new_reg = self.new_reg()
                self.emit(new_reg + " = call i64 " + fn + "(i64 " + old_reg + ", i64 " + val_reg + ")", is_in_func)
                self.emit_store_var(target.value, new_reg, is_in_func)
        elif node.type == "RETURN":
            self._flush_pending_native(is_in_func)
            if self.is_generator and node.children[0].type != "CONST_NONE":
                self.gen_expr(node.children[0], is_in_func)
            if self.is_generator:
                r_yv = self.new_reg()
                self.emit(r_yv + " = load i64, i64* %loc___yield_values", is_in_func)
                self.emit_ret(r_yv, is_in_func)
            else:
                val_reg = self.gen_expr(node.children[0], is_in_func)
                self.emit_ret(val_reg, is_in_func)
        elif node.type == "WHILE":
            self._flush_pending_native(is_in_func)
            do_else = len(node.children) > 2
            lbl_cond = self.new_label("while_cond")
            lbl_body = self.new_label("while_body")
            lbl_after = self.new_label("while_after")
            lbl_end = self.new_label("while_end")
            flag_var = ""
            if do_else:
                self.reg_counter = self.reg_counter + 1
                flag_var = "_w_brk_" + str(self.reg_counter)
                self.ensure_typed_alloca(flag_var, "i32", is_in_func)
                self.emit("store i32 0, i32* %loc_" + flag_var, is_in_func)
                self.loop_stack.append([lbl_after, lbl_cond, flag_var])
            else:
                self.loop_stack.append([lbl_end, lbl_cond, ""])

            self.emit_label(lbl_cond, is_in_func)
            cond_reg = self.gen_expr(node.children[0], is_in_func)
            c_i1 = self.emit_truthy_i1(cond_reg, is_in_func)
            if do_else:
                self.emit_cond_br(c_i1, lbl_body, lbl_after, is_in_func)
            else:
                self.emit_cond_br(c_i1, lbl_body, lbl_end, is_in_func)

            self.emit_label(lbl_body, is_in_func)
            self.gen_block_stmts(node.children[1], is_in_func)
            self.emit_br(lbl_cond, is_in_func)
            self.loop_stack.pop()

            if do_else:
                self.emit_label(lbl_after, is_in_func)
                r_flg = self.new_reg()
                self.emit(r_flg + " = load i32, i32* %loc_" + flag_var, is_in_func)
                r_no_brk = self.new_reg()
                self.emit(r_no_brk + " = icmp eq i32 " + r_flg + ", 0", is_in_func)
                lbl_else = self.new_label("while_else")
                self.emit_cond_br(r_no_brk, lbl_else, lbl_end, is_in_func)
                self.emit_label(lbl_else, is_in_func)
                self.gen_block_stmts(node.children[2], is_in_func)
                self.emit_br(lbl_end, is_in_func)

            self.emit_label(lbl_end, is_in_func)

        elif node.type == "FOR":
            self._flush_pending_native(is_in_func)
            var_name = node.value
            iter_node = node.children[0]
            do_else = len(node.children) > 2
            is_range_loop = False
            range_start = "0"
            range_stop = ""
            if iter_node.type == "CALL" and iter_node.children[0].type == "NAME" and iter_node.children[0].value == "range":
                rargs = iter_node.children[1].children
                if len(rargs) >= 1 and len(rargs) <= 2 and not do_else:
                    if len(rargs) == 1:
                        is_range_loop = True
                        range_stop = self._gen_range_bound(rargs[0], is_in_func)
                    elif len(rargs) == 2:
                        is_range_loop = True
                        range_start = self._gen_range_bound(rargs[0], is_in_func)
                        range_stop = self._gen_range_bound(rargs[1], is_in_func)
            if is_range_loop:
                body_node = node.children[1]
                var_types = {}
                var_types[var_name] = CodeGen.NATIVE_INT
                _idx = 0
                while _idx < len(self.local_vars):
                    _k = self.local_vars[_idx]
                    if _k in self._var_types:
                        if not (_k in var_types):
                            var_types[_k] = self._var_types[_k]
                    _idx = _idx + 1
                lhs_keys_list = self._scan_body_vars(body_node, var_types, var_name)
                if lhs_keys_list != False:
                    native_vars = []
                    native_int_vars = []
                    all_keys = [var_name]
                    a_idx = 0
                    while a_idx < len(lhs_keys_list):
                        k = lhs_keys_list[a_idx]
                        if k != var_name:
                            Found_k = False
                            fk_i = 0
                            while fk_i < len(all_keys):
                                if all_keys[fk_i] == k:
                                    Found_k = True
                                fk_i = fk_i + 1
                            if not Found_k:
                                all_keys.append(k)
                        a_idx = a_idx + 1
                    vk_idx = 0
                    while vk_idx < len(all_keys):
                        k = all_keys[vk_idx]
                        v = var_types[k]
                        if k != var_name and v == CodeGen.NATIVE_FLOAT:
                            native_vars.append(k)
                        if k != var_name and v == CodeGen.NATIVE_INT:
                            native_int_vars.append(k)
                        vk_idx = vk_idx + 1
                    for v in native_vars:
                        self.ensure_typed_alloca("_d_" + v, "double", is_in_func)
                        tv_r = self.emit_load_var(v, is_in_func)
                        dbl_r = self.new_reg()
                        self.emit(dbl_r + " = call double @to_double(i64 " + tv_r + ")", is_in_func)
                        self.emit("store double " + dbl_r + ", double* %loc__d_" + v, is_in_func)
                    for v in native_int_vars:
                        self.ensure_typed_alloca("_i_" + v, "i64", is_in_func)
                        tv_r = self.emit_load_var(v, is_in_func)
                        i_r = self.emit_unbox_int(tv_r, is_in_func)
                        self.emit("store i64 " + i_r + ", i64* %loc__i_" + v, is_in_func)
                    self.ensure_typed_alloca("_r_i", "i64", is_in_func)
                    self.emit("store i64 " + range_start + ", i64* %loc__r_i", is_in_func)
                    lbl_cond = self.new_label("nat_for_cond")
                    lbl_body = self.new_label("nat_for_body")
                    lbl_end = self.new_label("nat_for_end")
                    self.emit_label(lbl_cond, is_in_func)
                    r_ri = self.new_reg()
                    self.emit(r_ri + " = load i64, i64* %loc__r_i", is_in_func)
                    r_cmp = self.new_reg()
                    self.emit(r_cmp + " = icmp slt i64 " + r_ri + ", " + range_stop, is_in_func)
                    self.emit_cond_br(r_cmp, lbl_body, lbl_end, is_in_func)
                    self.emit_label(lbl_body, is_in_func)
                    s_idx = 0
                    while s_idx < len(body_node.children):
                        self._emit_native_stmt(body_node.children[s_idx], var_types, var_name, is_in_func)
                        s_idx = s_idx + 1
                    r_ri2 = self.new_reg()
                    self.emit(r_ri2 + " = load i64, i64* %loc__r_i", is_in_func)
                    r_rinext = self.new_reg()
                    self.emit(r_rinext + " = add i64 " + r_ri2 + ", 1", is_in_func)
                    self.emit("store i64 " + r_rinext + ", i64* %loc__r_i", is_in_func)
                    self.emit_br(lbl_cond, is_in_func)
                    self.emit_label(lbl_end, is_in_func)
                    self.pending_native_vars = [native_vars, native_int_vars, var_types, var_name]
                else:
                    self.reg_counter = self.reg_counter + 1
                    ri_var = "_rng_i_" + str(self.reg_counter)
                    self.ensure_typed_alloca(ri_var, "i64", is_in_func)
                    self.emit("store i64 " + range_start + ", i64* %loc_" + ri_var, is_in_func)
                    lbl_cond = self.new_label("rng_cond")
                    lbl_body = self.new_label("rng_body")
                    lbl_inc = self.new_label("rng_inc")
                    lbl_end = self.new_label("rng_end")
                    self.loop_stack.append([lbl_end, lbl_inc, ""])
                    self.emit_label(lbl_cond, is_in_func)
                    r_ri = self.new_reg()
                    self.emit(r_ri + " = load i64, i64* %loc_" + ri_var, is_in_func)
                    r_cmp = self.new_reg()
                    self.emit(r_cmp + " = icmp slt i64 " + r_ri + ", " + range_stop, is_in_func)
                    self.emit_cond_br(r_cmp, lbl_body, lbl_end, is_in_func)
                    self.emit_label(lbl_body, is_in_func)
                    r_boxed = self.emit_box_int(r_ri, is_in_func)
                    self.emit_store_var(var_name, r_boxed, is_in_func)
                    saved_rlv = None
                    if var_name in self.range_loop_vars:
                        saved_rlv = self.range_loop_vars[var_name]
                    self.range_loop_vars[var_name] = ri_var
                    self.gen_block_stmts(body_node, is_in_func)
                    if saved_rlv != None:
                        self.range_loop_vars[var_name] = saved_rlv
                    elif var_name in self.range_loop_vars:
                        del self.range_loop_vars[var_name]
                    self.emit_label(lbl_inc, is_in_func)
                    r_ri2 = self.new_reg()
                    self.emit(r_ri2 + " = load i64, i64* %loc_" + ri_var, is_in_func)
                    r_rinext = self.new_reg()
                    self.emit(r_rinext + " = add i64 " + r_ri2 + ", 1", is_in_func)
                    self.emit("store i64 " + r_rinext + ", i64* %loc_" + ri_var, is_in_func)
                    self.emit_br(lbl_cond, is_in_func)
                    self.loop_stack.pop()
                    self.emit_label(lbl_end, is_in_func)
            else:
                iter_reg = self.gen_expr(iter_node, is_in_func)
                flag_var = ""
                if do_else:
                    self.reg_counter = self.reg_counter + 1
                    flag_var = "_f_brk_" + str(self.reg_counter)
                    self.ensure_typed_alloca(flag_var, "i32", is_in_func)
                    self.emit("store i32 0, i32* %loc_" + flag_var, is_in_func)
                iter_state = self._emit_iter_header(iter_reg, is_in_func)
                r_elem = iter_state[0]
                lbl_inc = iter_state[3]
                lbl_end = iter_state[4]
                self.loop_stack.append([lbl_end, lbl_inc, flag_var])
                self.emit_store_var(var_name, r_elem, is_in_func)
                self.gen_block_stmts(node.children[1], is_in_func)
                self.loop_stack.pop()
                self._emit_iter_footer(iter_state, is_in_func)
                if do_else:
                    r_flg = self.new_reg()
                    self.emit(r_flg + " = load i32, i32* %loc_" + flag_var, is_in_func)
                    r_no_brk = self.new_reg()
                    self.emit(r_no_brk + " = icmp eq i32 " + r_flg + ", 0", is_in_func)
                    lbl_else = self.new_label("for_else")
                    lbl_done = self.new_label("for_done")
                    self.emit_cond_br(r_no_brk, lbl_else, lbl_done, is_in_func)
                    self.emit_label(lbl_else, is_in_func)
                    self.gen_block_stmts(node.children[2], is_in_func)
                    self.emit_br(lbl_done, is_in_func)
                    self.emit_label(lbl_done, is_in_func)
        elif node.type == "ASYNC_FOR":
            self.gen_stmt(ASTNode("FOR", node.value, node.children), is_in_func)
        elif node.type == "IF":
            self._flush_pending_native(is_in_func)
            cond_reg = self.gen_expr(node.children[0], is_in_func)
            c_i1 = self.emit_truthy_i1(cond_reg, is_in_func)
            lbl_then = self.new_label("if_then")
            lbl_else = self.new_label("if_else")
            lbl_end = self.new_label("if_end")
            if len(node.children) >= 3:
                self.emit_cond_br(c_i1, lbl_then, lbl_else, is_in_func)
                self.emit_label(lbl_then, is_in_func)
                self.gen_block_stmts(node.children[1], is_in_func)
                self.emit_br(lbl_end, is_in_func)
                self.emit_label(lbl_else, is_in_func)
                else_node = node.children[2]
                if else_node.type == "IF":
                    self.gen_stmt(else_node, is_in_func)
                else:
                    self.gen_block_stmts(else_node, is_in_func)
                self.emit_br(lbl_end, is_in_func)
                self.emit_label(lbl_end, is_in_func)
            else:
                self.emit_cond_br(c_i1, lbl_then, lbl_end, is_in_func)
                self.emit_label(lbl_then, is_in_func)
                self.gen_block_stmts(node.children[1], is_in_func)
                self.emit_br(lbl_end, is_in_func)
                self.emit_label(lbl_end, is_in_func)
        elif node.type == "RAISE":
            self._flush_pending_native(is_in_func)
            val_reg = self.gen_expr(node.children[0], is_in_func)
            self.emit("call void @turbo_raise(i64 " + val_reg + ")", is_in_func)
        elif node.type == "ASSERT":
            self._flush_pending_native(is_in_func)
            test_reg = self.gen_expr(node.children[0], is_in_func)
            c_i1 = self.emit_truthy_i1(test_reg, is_in_func)
            lbl_ok = self.new_label("assert_ok")
            lbl_fail = self.new_label("assert_fail")
            self.emit_cond_br(c_i1, lbl_ok, lbl_fail, is_in_func)
            self.emit_label(lbl_fail, is_in_func)
            if node.children[1].type == "CONST_NONE":
                self.emit("call void @turbo_assert_fail(i64 -1688849860263936)", is_in_func)
            else:
                msg_reg = self.gen_expr(node.children[1], is_in_func)
                self.emit("call void @turbo_assert_fail(i64 " + msg_reg + ")", is_in_func)
            self.emit_br(lbl_ok, is_in_func)
            self.emit_label(lbl_ok, is_in_func)
        elif node.type == "DEL":
            self._flush_pending_native(is_in_func)
            target = node.children[0]
            if target.type == "SUBSCRIPT":
                obj_reg = self.gen_expr(target.children[0], is_in_func)
                idx_reg = self.gen_expr(target.children[1], is_in_func)
                self.emit("call void @turbo_delitem(i64 " + obj_reg + ", i64 " + idx_reg + ")", is_in_func)
            elif target.type == "NAME":
                self.emit_store_var(target.value, "-1688849860263936", is_in_func)
        elif node.type == "GLOBAL" or node.type == "NONLOCAL":
            return
        elif node.type == "WITH":
            self._flush_pending_native(is_in_func)
            cm_reg = self.gen_expr(node.children[0], is_in_func)
            alias_node = node.children[1]
            body = node.children[2]
            self.reg_counter = self.reg_counter + 1
            uid = str(self.reg_counter)
            cm_var = "_with_cm_" + uid
            had_var = "_with_had_" + uid
            exc_var = "_with_exc_" + uid
            self.ensure_alloca(cm_var, is_in_func)
            self.ensure_typed_alloca(had_var, "i32", is_in_func)
            self.ensure_alloca(exc_var, is_in_func)
            self.emit("store i64 " + cm_reg + ", i64* %loc_" + cm_var, is_in_func)
            self.emit("store i32 0, i32* %loc_" + had_var, is_in_func)
            self.emit("store i64 -1688849860263936, i64* %loc_" + exc_var, is_in_func)

            sp_enter = self.get_str_ptr("__enter__", is_in_func)
            r_enter = self.new_reg()
            self.emit(r_enter + " = call i64 @turbo_call_method(i64 " + cm_reg + ", i8* " + sp_enter + ", i32 0, i64* null)", is_in_func)
            if alias_node.type == "NAME":
                self.emit_store_var(alias_node.value, r_enter, is_in_func)

            r_jb = self.new_reg()
            self.emit(r_jb + " = call i8* @turbo_try_push()", is_in_func)
            r_sj = self.new_reg()
            self.emit(r_sj + " = call i32 @setjmp(i8* " + r_jb + ")", is_in_func)
            r_is_exc = self.new_reg()
            self.emit(r_is_exc + " = icmp ne i32 " + r_sj + ", 0", is_in_func)
            lbl_try = self.new_label("with_try")
            lbl_catch = self.new_label("with_catch")
            lbl_after = self.new_label("with_after")
            self.emit_cond_br(r_is_exc, lbl_catch, lbl_try, is_in_func)

            self.emit_label(lbl_try, is_in_func)
            self.try_depth = self.try_depth + 1
            self.gen_block_stmts(body, is_in_func)
            self.try_depth = self.try_depth - 1
            self.emit("call void @turbo_try_pop()", is_in_func)
            self.emit_br(lbl_after, is_in_func)

            self.emit_label(lbl_catch, is_in_func)
            self.emit("call void @turbo_try_pop()", is_in_func)
            self.emit("store i32 1, i32* %loc_" + had_var, is_in_func)
            r_ge = self.new_reg()
            self.emit(r_ge + " = call i64 @turbo_get_exception()", is_in_func)
            self.emit("store i64 " + r_ge + ", i64* %loc_" + exc_var, is_in_func)
            self.emit_br(lbl_after, is_in_func)

            self.emit_label(lbl_after, is_in_func)
            r_had = self.new_reg()
            self.emit(r_had + " = load i32, i32* %loc_" + had_var, is_in_func)
            r_had_cmp = self.new_reg()
            self.emit(r_had_cmp + " = icmp ne i32 " + r_had + ", 0", is_in_func)
            lbl_whad = self.new_label("with_exc")
            lbl_wnorm = self.new_label("with_norm")
            lbl_wend = self.new_label("with_end")
            self.emit_cond_br(r_had_cmp, lbl_whad, lbl_wnorm, is_in_func)

            self.emit_label(lbl_whad, is_in_func)
            r_cm2 = self.new_reg()
            self.emit(r_cm2 + " = load i64, i64* %loc_" + cm_var, is_in_func)
            r_ev = self.new_reg()
            self.emit(r_ev + " = load i64, i64* %loc_" + exc_var, is_in_func)
            p_eargs = self.emit_args_array([r_ev, r_ev, "-1688849860263936"], is_in_func)
            sp_exit1 = self.get_str_ptr("__exit__", is_in_func)
            r_exres = self.new_reg()
            self.emit(r_exres + " = call i64 @turbo_call_method(i64 " + r_cm2 + ", i8* " + sp_exit1 + ", i32 3, i64* " + p_eargs + ")", is_in_func)
            c_supp = self.emit_truthy_i1(r_exres, is_in_func)
            lbl_reraise = self.new_label("with_reraise")
            self.emit_cond_br(c_supp, lbl_wend, lbl_reraise, is_in_func)
            self.emit_label(lbl_reraise, is_in_func)
            self.emit("call void @turbo_raise(i64 " + r_ev + ")", is_in_func)
            self.emit_br(lbl_wend, is_in_func)

            self.emit_label(lbl_wnorm, is_in_func)
            r_cm3 = self.new_reg()
            self.emit(r_cm3 + " = load i64, i64* %loc_" + cm_var, is_in_func)
            p_nargs = self.emit_args_array(["-1688849860263936", "-1688849860263936", "-1688849860263936"], is_in_func)
            sp_exit2 = self.get_str_ptr("__exit__", is_in_func)
            self.emit("call i64 @turbo_call_method(i64 " + r_cm3 + ", i8* " + sp_exit2 + ", i32 3, i64* " + p_nargs + ")", is_in_func)
            self.emit_br(lbl_wend, is_in_func)

            self.emit_label(lbl_wend, is_in_func)
        elif node.type == "ASYNC_WITH":
            self.gen_stmt(ASTNode("WITH", "", node.children), is_in_func)
        elif node.type == "TRY":
            self._flush_pending_native(is_in_func)
            body = node.children[0]
            handlers_node = node.children[1]
            else_body = node.children[2]
            finally_body = node.children[3]

            self.reg_counter = self.reg_counter + 1
            uid = str(self.reg_counter)
            had_var = "_try_had_" + uid
            hnd_var = "_try_hnd_" + uid
            exc_var = "_try_exc_" + uid
            self.ensure_typed_alloca(had_var, "i32", is_in_func)
            self.ensure_typed_alloca(hnd_var, "i32", is_in_func)
            self.ensure_alloca(exc_var, is_in_func)
            self.emit("store i32 0, i32* %loc_" + had_var, is_in_func)
            self.emit("store i32 0, i32* %loc_" + hnd_var, is_in_func)
            self.emit("store i64 -1688849860263936, i64* %loc_" + exc_var, is_in_func)

            r_jb = self.new_reg()
            self.emit(r_jb + " = call i8* @turbo_try_push()", is_in_func)
            r_sj = self.new_reg()
            self.emit(r_sj + " = call i32 @setjmp(i8* " + r_jb + ")", is_in_func)
            r_is_exc = self.new_reg()
            self.emit(r_is_exc + " = icmp ne i32 " + r_sj + ", 0", is_in_func)
            lbl_try = self.new_label("try_body")
            lbl_catch = self.new_label("try_catch")
            lbl_after = self.new_label("try_after")
            self.emit_cond_br(r_is_exc, lbl_catch, lbl_try, is_in_func)

            self.emit_label(lbl_try, is_in_func)
            self.try_depth = self.try_depth + 1
            self.gen_block_stmts(body, is_in_func)
            self.try_depth = self.try_depth - 1
            self.emit("call void @turbo_try_pop()", is_in_func)
            self.emit_br(lbl_after, is_in_func)

            self.emit_label(lbl_catch, is_in_func)
            self.emit("call void @turbo_try_pop()", is_in_func)
            self.emit("store i32 1, i32* %loc_" + had_var, is_in_func)
            r_ge = self.new_reg()
            self.emit(r_ge + " = call i64 @turbo_get_exception()", is_in_func)
            self.emit("store i64 " + r_ge + ", i64* %loc_" + exc_var, is_in_func)
            self.emit_br(lbl_after, is_in_func)

            self.emit_label(lbl_after, is_in_func)
            if len(handlers_node.children) > 0:
                r_had = self.new_reg()
                self.emit(r_had + " = load i32, i32* %loc_" + had_var, is_in_func)
                r_had_cmp = self.new_reg()
                self.emit(r_had_cmp + " = icmp ne i32 " + r_had + ", 0", is_in_func)
                lbl_dispatch = self.new_label("try_dispatch")
                lbl_post_h = self.new_label("try_post_h")
                self.emit_cond_br(r_had_cmp, lbl_dispatch, lbl_post_h, is_in_func)

                self.emit_label(lbl_dispatch, is_in_func)
                h_idx = 0
                while h_idx < len(handlers_node.children):
                    handler = handlers_node.children[h_idx]
                    exc_type = handler.children[0]
                    e_var_name = handler.value
                    handler_body = handler.children[1]
                    lbl_h_chk = self.new_label("try_h_chk")
                    lbl_h_run = self.new_label("try_h_run")
                    lbl_h_next = self.new_label("try_h_next")

                    r_hnd = self.new_reg()
                    self.emit(r_hnd + " = load i32, i32* %loc_" + hnd_var, is_in_func)
                    r_not_hnd = self.new_reg()
                    self.emit(r_not_hnd + " = icmp eq i32 " + r_hnd + ", 0", is_in_func)
                    self.emit_cond_br(r_not_hnd, lbl_h_chk, lbl_h_next, is_in_func)

                    self.emit_label(lbl_h_chk, is_in_func)
                    if exc_type.type == "CONST_NONE":
                        self.emit_br(lbl_h_run, is_in_func)
                    else:
                        et_reg = self.gen_expr(exc_type, is_in_func)
                        r_ev = self.new_reg()
                        self.emit(r_ev + " = load i64, i64* %loc_" + exc_var, is_in_func)
                        r_m = self.new_reg()
                        self.emit(r_m + " = call i32 @turbo_exception_matches(i64 " + r_ev + ", i64 " + et_reg + ")", is_in_func)
                        r_mcmp = self.new_reg()
                        self.emit(r_mcmp + " = icmp ne i32 " + r_m + ", 0", is_in_func)
                        self.emit_cond_br(r_mcmp, lbl_h_run, lbl_h_next, is_in_func)

                    self.emit_label(lbl_h_run, is_in_func)
                    self.emit("store i32 1, i32* %loc_" + hnd_var, is_in_func)
                    if e_var_name != "":
                        r_ev2 = self.new_reg()
                        self.emit(r_ev2 + " = load i64, i64* %loc_" + exc_var, is_in_func)
                        self.emit_store_var(e_var_name, r_ev2, is_in_func)
                    self.gen_block_stmts(handler_body, is_in_func)
                    self.emit_br(lbl_h_next, is_in_func)

                    self.emit_label(lbl_h_next, is_in_func)
                    h_idx = h_idx + 1

                r_hnd_fin = self.new_reg()
                self.emit(r_hnd_fin + " = load i32, i32* %loc_" + hnd_var, is_in_func)
                r_unh = self.new_reg()
                self.emit(r_unh + " = icmp eq i32 " + r_hnd_fin + ", 0", is_in_func)
                lbl_unh = self.new_label("try_unhandled")
                self.emit_cond_br(r_unh, lbl_unh, lbl_post_h, is_in_func)
                self.emit_label(lbl_unh, is_in_func)
                r_ev_unh = self.new_reg()
                self.emit(r_ev_unh + " = load i64, i64* %loc_" + exc_var, is_in_func)
                self.emit("call void @turbo_raise(i64 " + r_ev_unh + ")", is_in_func)
                self.emit_br(lbl_post_h, is_in_func)
                self.emit_label(lbl_post_h, is_in_func)

            if else_body.type != "CONST_NONE":
                r_had2 = self.new_reg()
                self.emit(r_had2 + " = load i32, i32* %loc_" + had_var, is_in_func)
                r_no_exc = self.new_reg()
                self.emit(r_no_exc + " = icmp eq i32 " + r_had2 + ", 0", is_in_func)
                lbl_else = self.new_label("try_else")
                lbl_after_else = self.new_label("try_after_else")
                self.emit_cond_br(r_no_exc, lbl_else, lbl_after_else, is_in_func)
                self.emit_label(lbl_else, is_in_func)
                self.gen_block_stmts(else_body, is_in_func)
                self.emit_br(lbl_after_else, is_in_func)
                self.emit_label(lbl_after_else, is_in_func)

            if finally_body.type != "CONST_NONE":
                self.gen_block_stmts(finally_body, is_in_func)

            if len(handlers_node.children) == 0:
                r_had3 = self.new_reg()
                self.emit(r_had3 + " = load i32, i32* %loc_" + had_var, is_in_func)
                r_re = self.new_reg()
                self.emit(r_re + " = icmp ne i32 " + r_had3 + ", 0", is_in_func)
                lbl_re = self.new_label("try_fin_re")
                lbl_fin_end = self.new_label("try_fin_end")
                self.emit_cond_br(r_re, lbl_re, lbl_fin_end, is_in_func)
                self.emit_label(lbl_re, is_in_func)
                r_ev3 = self.new_reg()
                self.emit(r_ev3 + " = load i64, i64* %loc_" + exc_var, is_in_func)
                self.emit("call void @turbo_raise(i64 " + r_ev3 + ")", is_in_func)
                self.emit_br(lbl_fin_end, is_in_func)
                self.emit_label(lbl_fin_end, is_in_func)
        elif node.type == "MATCH":
            self._flush_pending_native(is_in_func)
            subj_reg = self.gen_expr(node.children[0], is_in_func)
            lbl_end = self.new_label("match_end")
            n = len(node.children)
            i = 1
            while i < n:
                case_node = node.children[i]
                pattern_node = case_node.children[0]
                body_node = case_node.children[1]
                lbl_body = self.new_label("case_body")
                lbl_next = self.new_label("case_next")
                if pattern_node.type == "PATTERN_LIT":
                    lit_reg = self.gen_expr(pattern_node.children[0], is_in_func)
                    r_eq = self.new_reg()
                    self.emit(r_eq + " = call i64 @turbo_eq(i64 " + subj_reg + ", i64 " + lit_reg + ")", is_in_func)
                    c_i1 = self.emit_truthy_i1(r_eq, is_in_func)
                    self.emit_cond_br(c_i1, lbl_body, lbl_next, is_in_func)
                else:
                    self.emit_br(lbl_body, is_in_func)
                self.emit_label(lbl_body, is_in_func)
                if pattern_node.type == "PATTERN_CAP":
                    self.emit_store_var(pattern_node.value, subj_reg, is_in_func)
                self.gen_block_stmts(body_node, is_in_func)
                self.emit_br(lbl_end, is_in_func)
                self.emit_label(lbl_next, is_in_func)
                i = i + 1
            self.emit_label(lbl_end, is_in_func)
        elif node.type == "DEF" or node.type == "ASYNC_DEF":
            self._flush_pending_native(is_in_func)
            self.gen_func_def(node, "", "", False)
        elif node.type == "CLASS":
            self._flush_pending_native(is_in_func)
            self.gen_class_def(node, "")

    def build_llvm_ir(self):
        out = []
        # Compact LLVM Struct Types for heap objects
        out.append("%String = type { i32, i32, i8* }")
        out.append("%List = type { i32, i32, i32, i32, i64* }")
        out.append("%Dict = type { i32, i32, i32, i32, i64*, i64* }")
        out.append("%Instance = type { i32, i32, i64, %Dict* }")
        out.append("")

        # External runtime declarations
        out.append("declare void @turbo_init_args(i32, i8**)")
        out.append("declare i64 @make_int_from_ll(i64)")
        out.append("declare i64 @make_int(i8*)")
        out.append("declare i64 @make_float(double)")
        out.append("declare i64 @make_complex(double, double)")
        out.append("declare i64 @make_bytes_from_str(i8*)")
        out.append("declare i64 @make_str(i8*)")
        out.append("declare i64 @make_bool(i32)")
        out.append("declare i64 @make_list()")
        out.append("declare i64 @make_tuple()")
        out.append("declare i64 @make_set()")
        out.append("declare i64 @make_dict()")
        out.append("declare i64 @make_func(i64 (i32, i64*)*, i8*)")
        out.append("declare i64 @make_staticmethod(i64 (i32, i64*)*, i8*)")
        out.append("declare i64 @make_classmethod(i64 (i32, i64*)*, i8*)")
        out.append("declare i64 @make_property(i64 (i32, i64*)*, i64 (i32, i64*)*, i8*)")
        out.append("declare i64 @make_class(i8*)")
        out.append("declare i64 @make_module()")
        out.append("declare void @turbo_module_set(i64, i8*, i64)")
        out.append("declare i64 @turbo_module_get(i64, i8*)")
        out.append("declare void @turbo_class_add_method(i64, i8*, i64 (i32, i64*)*)")
        out.append("declare void @turbo_class_set_attr(i64, i8*, i64)")
        out.append("declare void @turbo_list_append(i64, i64)")
        out.append("declare void @turbo_tuple_append(i64, i64)")
        out.append("declare void @turbo_set_add(i64, i64)")
        out.append("declare i64 @turbo_add(i64, i64)")
        out.append("declare i64 @turbo_sub(i64, i64)")
        out.append("declare i64 @turbo_mul(i64, i64)")
        out.append("declare i64 @turbo_div(i64, i64)")
        out.append("declare i64 @turbo_floordiv(i64, i64)")
        out.append("declare i64 @turbo_mod(i64, i64)")
        out.append("declare i64 @turbo_pow(i64, i64)")
        out.append("declare i64 @turbo_matmul(i64, i64)")
        out.append("declare i64 @turbo_bitand(i64, i64)")
        out.append("declare i64 @turbo_bitor(i64, i64)")
        out.append("declare i64 @turbo_bitxor(i64, i64)")
        out.append("declare i64 @turbo_bitnot(i64)")
        out.append("declare i64 @turbo_lshift(i64, i64)")
        out.append("declare i64 @turbo_rshift(i64, i64)")
        out.append("declare i64 @turbo_eq(i64, i64)")
        out.append("declare i64 @turbo_ne(i64, i64)")
        out.append("declare i64 @turbo_lt(i64, i64)")
        out.append("declare i64 @turbo_gt(i64, i64)")
        out.append("declare i64 @turbo_lte(i64, i64)")
        out.append("declare i64 @turbo_gte(i64, i64)")
        out.append("declare i32 @turbo_in(i64, i64)")
        out.append("declare i32 @turbo_is_truthy(i64)")
        out.append("declare i64 @turbo_not(i64)")
        out.append("declare i64 @turbo_call(i64, i32, i64*)")
        out.append("declare i64 @turbo_call_method(i64, i8*, i32, i64*)")
        out.append("declare i64 @turbo_getattr(i64, i8*)")
        out.append("declare void @turbo_setattr(i64, i8*, i64)")
        out.append("declare i64 @turbo_getitem(i64, i64)")
        out.append("declare void @turbo_setitem(i64, i64, i64)")
        out.append("declare void @turbo_delitem(i64, i64)")
        out.append("declare i64 @turbo_slice(i64, i64, i64)")
        out.append("declare i64 @turbo_len(i64)")
        out.append("declare i64 @turbo_str(i64)")
        out.append("declare i64 @turbo_list(i32, i64*)")
        out.append("declare i64 @ int_to_ll(i64)".replace("@ ", "@"))
        out.append("declare double @to_double(i64)")
        out.append("declare void @turbo_raise(i64)")
        out.append("declare void @turbo_assert_fail(i64)")
        out.append("declare i32 @turbo_exception_matches(i64, i64)")
        out.append("declare i8* @turbo_try_push()")
        out.append("declare void @turbo_try_pop()")
        out.append("declare i64 @turbo_get_exception()")
        out.append("declare i32 @setjmp(i8*) returns_twice")
        out.append("")

        out.append("@_cached_str_empty = external global i64")
        b_i = 0
        while b_i < len(self.builtins_list):
            bname = self.builtins_list[b_i]
            out.append("@t_" + bname + " = external global i64")
            b_i = b_i + 1
        out.append("")

        s_i = 0
        while s_i < len(self.str_defs):
            out.append(self.str_defs[s_i])
            s_i = s_i + 1
        if len(self.str_defs) > 0:
            out.append("")

        r_i = 0
        while r_i < len(self.raw_globals):
            out.append(self.raw_globals[r_i])
            r_i = r_i + 1

        g_i = 0
        while g_i < len(self.global_vars):
            gname = self.global_vars[g_i]
            if not self.is_builtin_name(gname):
                out.append("@t_" + gname + " = global i64 -1688849860263936")
            g_i = g_i + 1
        out.append("")

        f_i = 0
        while f_i < len(self.funcs_list):
            out.append(self.funcs_list[f_i])
            out.append("")
            f_i = f_i + 1

        out.append("define void @turbo_main() {")
        out.append("entry:")
        a_i = 0
        while a_i < len(self.main_allocas):
            aname = self.main_allocas[a_i]
            out.append("  %loc_" + aname + " = alloca i64")
            out.append("  store i64 -1688849860263936, i64* %loc_" + aname)
            a_i = a_i + 1
        a_i = 0
        while a_i < len(self.main_typed_allocas):
            tinfo = self.main_typed_allocas[a_i]
            out.append("  %loc_" + tinfo[0] + " = alloca " + tinfo[1])
            a_i = a_i + 1
        a_i = 0
        while a_i < len(self.main_sized_allocas):
            sinfo = self.main_sized_allocas[a_i]
            out.append("  %loc_" + sinfo[0] + " = alloca [" + str(sinfo[1]) + " x i64]")
            a_i = a_i + 1
        m_i = 0
        while m_i < len(self.main_lines):
            out.append(self.main_lines[m_i])
            m_i = m_i + 1
        out.append("  ret void")
        out.append("}")
        out.append("")

        out.append("define i32 @main(i32 %argc, i8** %argv) {")
        out.append("entry:")
        out.append("  call void @turbo_init_args(i32 %argc, i8** %argv)")
        out.append("  call void @turbo_main()")
        out.append("  ret i32 0")
        out.append("}")
        out.append("")
        return "\n".join(out)

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
                output_file = sys_argv[i + 1]
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
    g_idx = 0
    while g_idx < len(globals_list):
        g_name = globals_list[g_idx]
        if not cg.is_builtin_name(g_name):
            cg.add_global(g_name)
        g_idx = g_idx + 1

    cg.gen_block_stmts(module_ast, False)
    llvm_source = cg.build_llvm_ir()

    f_out = open(output_file, "w")
    f_out.write(llvm_source)
    f_out.close()

main()
