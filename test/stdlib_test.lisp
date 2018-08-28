(load "stdlib.lisp")

(fun {test name expected body} {
     (= {got} (eval body))
     (if (!= got expected) {
        (print "test" name "fails")
        (error "test fails")
        })
     })

(test "min" -99 {min $ list 1 2 3 -99 4 5 6})
(test "max" 100 {max $ list 1 2 3 100 4 5 6})
