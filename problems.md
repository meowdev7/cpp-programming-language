20 Problems ->

**Lexer (5 issues):**
- No escape sequences in strings (\n, \t)
- No negative number literals (-5) -> being treated as MINUS and NUMBER 5
- No compound assignment operators (+=, -=, etc.)
- Division vs comment issue

**Parser (7 issues):**
- No else clause for if statements
- No for loop (keyword exists but not implemented yet)
- No break/continue in loops
- No && and || logical operators
- No unary plus operator
- Function parameters always hardcoded to int -> major issue
- Function return type always int -> major issue

**Interpreter (4 issues):**
- No garbage collection (memory leaks) -> can be done later since string manipulation etc has not been implemented yet  
- No type coercion
- No null handling -> need a NULL token and runtime value
- No explicit global variable declaration

**Language Design (4 issues):**
- No modules/imports 
- No standard library -> specially for built-in features like print 
- Improper error recovery (stops on first error) -> should show all the errors 
- No runtime stack traces -> division by zero?