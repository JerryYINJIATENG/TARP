\ENCODING=ISO-8859-1
\Problem name: smkp_2_mean

Minimize
 obj: 0.5 theta{SCEN1} + 50 x0 + 91 x1 + 0 slack{7} + 0 slack{8} + 0 slack{9}
      + 0 slack{10} + 0 slack{11} + 0 slack{12}
Subject To
 _esti_cons0#0: theta{SCEN1} - 23 y0{SCEN1} - 41 y1{SCEN1} - 20 y2{SCEN1}
                - 69 y3{SCEN1}  = 0
 c1#1:          79 x0 + 50 x1 >= 45
 c2#2:          46 x0 + x1 >= 1
 c3_SCEN1#3:    88 x0 + 87 y0{SCEN1} + 83 y1{SCEN1} + 66 y2{SCEN1}
                + 24 y3{SCEN1} >= 22
 c4_SCEN1#4:    40 x0 + 52 y0{SCEN1} + 94 y1{SCEN1} + 54 y2{SCEN1}
                + 84 y3{SCEN1} >= 14
 c5_SCEN1#5:    32 x1 + 62 y0{SCEN1} + 91 y1{SCEN1} + 38 y2{SCEN1}
                + 100 y3{SCEN1} >= 25
 c6_SCEN1#6:    14 x1 + 88 y0{SCEN1} + 25 y1{SCEN1} + 24 y2{SCEN1}
                + 12 y3{SCEN1} >= 6
Bounds
      theta{SCEN1} Free
 0 <= x0 <= 1
 0 <= x1 <= 1
 0 <= y0{SCEN1} <= 1
 0 <= y1{SCEN1} <= 1
 0 <= y2{SCEN1} <= 1
 0 <= y3{SCEN1} <= 1
Generals
 x0  x1  y0{SCEN1}  y1{SCEN1}  y2{SCEN1}  y3{SCEN1} 
End
