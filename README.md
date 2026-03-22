## Hydro Grammar

```
[prog]    -> { [stmt] }*

[stmt]    -> exit ( [expr] ) ;
           | let ident = [expr] ;

[expr]    -> [addExpr]

[addExpr] -> [mulExpr]
           | [addExpr] + [mulExpr]   (precedence 0)
           | [addExpr] - [mulExpr]   (precedence 0)

[mulExpr] -> [primary]
           | [mulExpr] * [primary]   (precedence 1)
           | [mulExpr] / [primary]   (precedence 1)

[primary] -> int_lit
           | ident
           | ( [expr] )              (highest precedence)
```