## Hydro Grammar

```
[prog]    -> { [stmt] }*

[stmt]    -> exit ( [expr] ) ;
           | let ident = [expr] ;

[expr]    -> [term]
           | [binExpr]

[term]    -> int_lit
           | ident

[binExpr] -> [expr] * [expr]   (precedence 1)
           | [expr] / [expr]   (precedence 1)
           | [expr] + [expr]   (precedence 0)
           | [expr] - [expr]   (precedence 0)
```