//# 4 inputs
//# 1 outputs
//# 3 D-type flipflops
//# 2 inverters
//# 8 gates (1 ANDs + 1 NANDs + 2 ORs + 4 NORs)

module s27(CK,G0,G1,G17,G2,G3);
input CK,G0,G1,G2,G3;
output G17;

  wire G5,G10,G6,G11,G7,G13,G14,G8,G15,G12,G16,G9;

  DFF_X2 DFF_0( .CK(CK), .Q(G5), .D(G10) );
  DFF_X2 DFF_1( .CK(CK), .Q(G6), .D(G11) );
  DFF_X2 DFF_2( .CK(CK), .Q(G7), .D(G13) );
  INV_X1 NOT_0( .ZN(G14), .A(G0) );
  INV_X1 NOT_1( .ZN(G17), .A(G11) );
  AND2_X1 AND2_0( .ZN(G8), .A1(G14), .A2(G6) );
  OR2_X1 OR2_0( .ZN(G15), .A1(G12), .A2(G8) );
  OR2_X1 OR2_1( .ZN(G16), .A1(G3), .A2(G8) );
  NAND2_X1 NAND2_0( .ZN(G9), .A1(G16), .A2(G15) );
  NOR2_X1 NOR2_0( .ZN(G10), .A1(G14), .A2(G11) );
  NOR2_X2 NOR2_1( .ZN(G11), .A1(G5), .A2(G9) );
  NOR2_X2 NOR2_2( .ZN(G12), .A1(G1), .A2(G7) );
  NOR2_X1 NOR2_3( .ZN(G13), .A1(G2), .A2(G12) );

endmodule
