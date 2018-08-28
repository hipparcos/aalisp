(fun {min xs} {
     fold (\ {x y} {if (< x y) {x} {y}}) (head xs) xs
     })

(fun {max xs} {
     fold (\ {x y} {if (> x y) {x} {y}}) (head xs) xs
     })

{min max}
