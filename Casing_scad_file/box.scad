
m_x = 130;
m_y = 64;
m_z = 32;

difference(){
    cube([m_x,m_y,m_z]);
    translate([-10,5,5])cube([m_x+20,m_y-10,m_z-10],0);
    translate([5,5,5])cube([m_x,m_y-10,m_z-0],0);
    translate([2,5,25])cube([5,m_y-10,10],0);
}
//translate([m_x-5,0,0])cube([5,m_y,m_z]);

//38,50,20.9 size sensor
X_offset = 0;
Y_offset = 4;
Z_offset = 5;
difference(){
    translate([X_offset+0,Y_offset+0,Z_offset+0])cube([44,56,22]);
    translate([X_offset+2,Y_offset+2,Z_offset+2])cube([40,52,21]);
    translate([X_offset+-1,Y_offset+28,Z_offset+2])cube([4,24,18]);
    translate([X_offset+-1,Y_offset+3,Z_offset+2])cube([4,20,5]);
    translate([X_offset+42,Y_offset+25,Z_offset+7])cube([5,26,8]);
    
}

//41,33 size charger panel





uc_x = 88;
uc_y = 58;
uc_z = 27;

c_X_offset = 0;
c_Y_offset = 4;
c_Z_offset = 5;
temp=50;


difference(){
    translate([m_x-uc_x,(m_y/2)-(uc_y/2),c_Z_offset])cube([uc_x,uc_y,uc_z]);
    translate([m_x-uc_x+2,(m_y/2)+2-(uc_y/2),c_Z_offset+2])cube([uc_x-4,uc_y-4,uc_z-1]);
    translate([X_offset+42,Y_offset+25,Z_offset+7])cube([5,26,8]);
    translate([m_x-uc_x + 85,(m_y/2)-(uc_y/2)+25,c_Z_offset+20])cube([10,10,10]);
    
}

difference(){
    translate([0,100,0])cube([m_x,m_y,2]);
    translate([m_x-uc_x+8,100+8,-3])cylinder(h=6, r=1.5);
    translate([m_x-uc_x+58,100+8,-3])cylinder(h=6, r=1.5);
    translate([m_x-uc_x-8+58,100+7+8,-3])cylinder(h=6, r=4);
    translate([m_x-uc_x+58,100+38,-3])cylinder(h=6, r=1.5);
}

