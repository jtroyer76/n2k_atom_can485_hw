// n2k_atom_can485 enclosure: a base and a lid that clamp the board by its edges.
// OpenSCAD 2021.01 or later. Units mm.
//
// Coordinates: X = KiCad board x, Y = 64 - KiCad board y (board origin at its
// top-left corner), Z = 0 at the board's top face. The terminal end is Y = 64,
// the USB end Y = 0. Part positions come from the KiCad layout, part sizes from
// the datasheets and Seeed's XIAO ESP32-S3 3D model; see README.md.
//
// Export: set part = "base" or "lid" (both come out in print orientation).

part = "assembly"; // [assembly, base, lid]
explode = 12;      // assembly view: lift the lid by this much

/* [Board] */
brd = [32, 64, 1.6];

/* [Fit] */
c = 0.25;          // board to side wall, each side
cy = 0.15;         // board end to corner hook
end_recess = 0.3;  // case end faces sit this far inside the board ends, so the plug and USB cable seat on their connectors
seam_gap = 0.2;    // base walls stop this far below the board top, so the lid clamps the board

/* [Shell] */
wall = 2.0;
floor_t = 1.6;
top_t = 1.6;
under = 3.0;       // space under the board: J1 pin tails (3.5 mm from the body per the drawing) and solder
ceiling = 8.5;     // space above the board: J1 is the tallest part (7.1 / 7.5 mm, see README)
corner_r = 1.5;
ledge = 1.0;       // board rests on ledges this far in from its long edges
end_wall = 1.5;    // end walls: base ones carry the board, lid ones press on it
rib = 0.8;         // lid ribs press on the board this far in from its long edges
rib_h = 1.0;
hook_t = 1.2;      // corner hooks past the board ends hold it in Y

/* [Screws: M2.5 x 10 from below into the lid] */
screw_clear = 2.9;
screw_head_d = 5.5;
screw_head_z = 3.0;  // counterbore depth from the bottom face
screw_pilot = 2.2;   // thread-forming into the lid; use the insert's hole size for heat-set inserts
screw_depth = 8;
boss_d = 7;
boss_out = 2.4;      // boss centre outside the board edge
boss_y = [5, 59];

/* [Mounting tabs] */
mount_tabs = true;
tab_len = 10;
tab_w = 10;
tab_t = 3;
tab_hole = 4.5;

/* [Labels] */
labels = true;
label_depth = 0.4;

/* [Hidden] */
$fn = 48;
eps = 0.01;

// Derived shell extents
z_bot = -brd.z - under - floor_t;
z_top = ceiling + top_t;
x0 = -c - wall;
x1 = brd.x + c + wall;
y0 = end_recess;
y1 = brd.y - end_recess;
boss_x = [-c - boss_out, brd.x + c + boss_out];

// Parts on the board (from the KiCad layout, converted to these coordinates)
j1_x = [3.3, 28.7];          // MX15EDGRC-3.5-07P body, 25.4 mm wide
j1_y = [54.8, 64];           // 9.2 mm deep, front face on the board edge
j1_h = 7.5;                  // drawing shows 7.10 and 7.50 in two views; the larger is used
j1_pin_x = [for (i = [0:6]) 5.5 + 3.5 * i];
plug_x = [3.2, 28.8];        // MX15EDGK-3.5-07P plug, 25.6 mm wide, centred on the pins
xiao_x = [7.06, 24.94];
xiao_y = [1.43, 22.52];
xiao_t = 1.25;               // XIAO PCB, from Seeed's 3D model
xiao_clear = 1.6;            // lid stays this high over the XIAO end (PCB + solder + margin)
usb_cx = 16.0;
usb_w = 8.94;
usb_z = [1.25, 4.46];        // USB-C receptacle above the carrier's top face
usb_face_y = -0.1;
shield_h = 3.24;             // XIAO shield top above the carrier
led_user = [10.11, 5.29];    // XIAO user LED (GPIO21)
btn_boot = [10.16, 3.09];    // XIAO BOOT (GPIO0)
btn_reset = [21.84, 3.09];   // XIAO RESET (EN)
jp2_y = [46.1, 49.5];        // RS485 termination jumper, 0.8 mm from the right edge

// Openings
j1_open = [j1_x[0] - 0.4, j1_x[1] + 0.4];
usb_open_w = usb_w + 1.0;
usb_open_top = usb_z[1] + 0.5;
hook_x_j1 = [plug_x[0] - 0.8, plug_x[1] + 0.8];  // hooks stay clear of the plug
hook_x_usb = [6.0, 26.0];                         // and of a USB-C plug overmould
hole_d = 1.6;                // LED and button holes (press the buttons with a pin)
tube_d = 3.4;
tube_bot = usb_open_top;     // tubes end above the USB receptacle

module rbox(xa, ya, xb, yb, za, zb, r) {
    hull() for (x = [xa + r, xb - r], y = [ya + r, yb - r])
        translate([x, y, za]) cylinder(r = r, h = zb - za);
}

module box(p0, p1) { translate(p0) cube(p1 - p0); }

module base() {
    difference() {
        union() {
            rbox(x0, y0, x1, y1, z_bot, -seam_gap, corner_r);
            for (x = boss_x, y = boss_y)
                translate([x, y, z_bot]) cylinder(d = boss_d, h = -seam_gap - z_bot);
            // Corner hooks past both board ends
            for (xr = [[x0, hook_x_usb[0]], [hook_x_usb[1], x1]])
                box([xr[0], -cy - hook_t, z_bot], [xr[1], y0 + eps, -seam_gap]);
            for (xr = [[x0, hook_x_j1[0]], [hook_x_j1[1], x1]])
                box([xr[0], y1 - eps, z_bot], [xr[1], brd.y + cy + hook_t, -seam_gap]);
            if (mount_tabs)
                for (s = [0, 1]) {
                    xa = s == 0 ? x0 - tab_len : x1 - corner_r;
                    xb = s == 0 ? x0 + corner_r : x1 + tab_len;
                    rbox(xa, brd.y / 2 - tab_w / 2, xb, brd.y / 2 + tab_w / 2, z_bot, z_bot + tab_t, 2);
                }
        }
        // Board slot, open at both ends between the hooks
        box([-c, -cy, -brd.z], [brd.x + c, brd.y + cy, 1]);
        // Space under the board, inside the ledges and end supports
        box([ledge, y0 + end_wall, z_bot + floor_t], [brd.x - ledge, y1 - end_wall, -brd.z + eps]);
        // Screws from below, heads in counterbores
        for (x = boss_x, y = boss_y) {
            translate([x, y, z_bot - 1]) cylinder(d = screw_clear, h = 20);
            translate([x, y, z_bot - 1]) cylinder(d = screw_head_d, h = screw_head_z + 1);
        }
        if (mount_tabs)
            for (x = [x0 - tab_len / 2, x1 + tab_len / 2])
                translate([x, brd.y / 2, z_bot - 1]) cylinder(d = tab_hole, h = tab_t + 2);
    }
}

module lid_shell() {
    difference() {
        union() {
            rbox(x0, y0, x1, y1, 0, z_top, corner_r);
            for (x = boss_x, y = boss_y)
                translate([x, y, 0]) cylinder(d = boss_d, h = z_top);
        }
        // Cavity above the ribs, then between them
        box([-c, y0 + end_wall, rib_h], [brd.x + c, y1 - end_wall, ceiling]);
        box([rib, y0 + end_wall, -1], [brd.x - rib, y1 - end_wall, rib_h + eps]);
        // Rib relief over JP2 (a closed jumper has a solder blob)
        box([brd.x - rib - 1, jp2_y[0] - 0.6, -1], [brd.x + c, jp2_y[1] + 0.6, rib_h + eps]);
        // Terminal end: J1 fills this opening, the plug seats against J1
        box([j1_open[0], y1 - end_wall - 1, -1], [j1_open[1], y1 + 1, ceiling]);
        // USB end: clear of the XIAO PCB, open for the receptacle
        box([xiao_x[0] - 0.6, y0 - 1, -1], [xiao_x[1] + 0.6, y0 + end_wall + 1, xiao_clear]);
        box([usb_cx - usb_open_w / 2, y0 - 1, -1], [usb_cx + usb_open_w / 2, y0 + end_wall + 1, usb_open_top]);
        // Screw pilots
        for (x = boss_x, y = boss_y)
            translate([x, y, -1]) cylinder(d = screw_pilot, h = screw_depth + 1);
    }
}

module lid() {
    difference() {
        union() {
            lid_shell();
            for (p = [led_user, btn_boot, btn_reset])
                translate([p.x, p.y, tube_bot]) cylinder(d = tube_d, h = ceiling - tube_bot + eps);
        }
        for (p = [led_user, btn_boot, btn_reset])
            translate([p.x, p.y, tube_bot - 1]) cylinder(d = hole_d, h = z_top);
        if (labels) translate([0, 0, z_top - label_depth]) linear_extrude(label_depth + 1) lid_labels();
    }
}

module lid_labels() {
    // Terminal pins, read from above with the terminal end away from you
    pin_names = ["+", "-", "H", "L", "A", "B", "G"];
    for (i = [0:6])
        translate([j1_pin_x[i], 60.2]) text(pin_names[i], size = 2.6, halign = "center", valign = "center");
    translate([(j1_pin_x[0] + j1_pin_x[3]) / 2, 56.2]) text("N2K", size = 2.2, halign = "center", valign = "center");
    translate([(j1_pin_x[4] + j1_pin_x[6]) / 2, 56.2]) text("RS485", size = 2.2, halign = "center", valign = "center");
    translate([btn_boot.x - 2.2, btn_boot.y]) text("BOOT", size = 1.8, halign = "right", valign = "center");
    translate([led_user.x - 2.2, led_user.y]) text("LED", size = 1.8, halign = "right", valign = "center");
    translate([btn_reset.x + 2.2, btn_reset.y]) text("RST", size = 1.8, halign = "left", valign = "center");
}

// Simplified board and parts, for the assembly view only
module board_model() {
    color("darkgreen") box([0, 0, -brd.z], [brd.x, brd.y, 0]);
    color("limegreen") box([j1_x[0], j1_y[0], 0], [j1_x[1], j1_y[1], j1_h]);
    color("dimgray") box([xiao_x[0], xiao_y[0], 0], [xiao_x[1], xiao_y[1], xiao_t]);
    color("silver") box([xiao_x[0] + 2, xiao_y[0] + 7, xiao_t], [xiao_x[1] - 2, xiao_y[1] - 1, shield_h]);
    color("silver") box([usb_cx - usb_w / 2, usb_face_y, usb_z[0]], [usb_cx + usb_w / 2, usb_face_y + 7.3, usb_z[1]]);
}

if (part == "base") {
    translate([0, 0, -z_bot]) base();
} else if (part == "lid") {
    translate([0, 0, z_top]) rotate([0, 180, 0]) translate([-brd.x, 0, 0]) lid();
} else {
    color("lightgray") base();
    board_model();
    color("lightsteelblue", 0.6) translate([0, 0, explode]) lid();
}
