Author: Đặng Kiến Quốc - 2114568

# Run custom node

To run `my-custom-node`, it must be placed in the 6lrb/examples directory.

This means it should be at the same level as the examples in the examples folder (since the path hasn't been modified to allow making the file from a different location).

# Custom favicon for web

Open file `webserver-main.c` in path `6lbr/examples/6lbr/apps/6lbr-webserver`. Then add this code to thread generate index

```
    add("<head>\n");
    add("<link rel=\"icon\" type=\"image/png\" href=\"https://hcmut.edu.vn/img/nhanDienThuongHieu/01_logobachkhoatoi.png\">\n");
    add("</head>\n");
```
