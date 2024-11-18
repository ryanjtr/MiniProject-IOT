#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MAXBUF 100

static int s_status;
static struct sockaddr_in6 s_sin6;
static socklen_t s_sin6len = sizeof(struct sockaddr_in6);
static char s_buffer[MAXBUF];
static char target_address[INET6_ADDRSTRLEN] = ""; // Địa chỉ mặc định

void display_menu()
{
    printf("\n----------Command options----------\n");
    printf("1. led green on\n");
    printf("2. led green off\n");
    printf("3. led red on\n");
    printf("4. led red off\n");
    printf("5. led blue on\n");
    printf("6. led blue off\n");
    printf("7. all led on\n");
    printf("8. all led off\n");
    printf("9. get network status\n");
    printf("10. get green led status\n");
    printf("11. get red led status\n");
    printf("12. get blue led status\n");
    printf("-----------------------------------\n");
    printf("13. Set transmission address\n");
    printf("14. Help\n");
    printf("15. Quit\n");
    printf("-----------------------------------\n\n");
}

void execute_command(const char *command)
{
    if (strcmp(target_address, "") == 0)
    {
        printf("Please set transmission address first\n");
        return;
    }
    int sock;
    int status;
    struct addrinfo sainfo, *psinfo;
    struct sockaddr_in6 sin6;
    int sin6len;
    char buffer[MAXBUF];

    sin6len = sizeof(struct sockaddr_in6); // Gán kích thước địa chỉ IPv6 cho sin6len.
    // Copy command into buffer
    snprintf(buffer, sizeof(buffer), "%s", command);

    // Create UDP socket
    sock = socket(PF_INET6, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("socket");
        return; // Quay lại menu nếu tạo socket không thành công
    }

    // Bind to any local address
    memset(&sin6, 0, sizeof(struct sockaddr_in6));
    sin6.sin6_port = htons(3000);
    sin6.sin6_family = AF_INET6;
    sin6.sin6_addr = in6addr_any;

    status = bind(sock, (struct sockaddr *)&sin6, s_sin6len);
    if (status == -1)
    {
        perror("bind");
        close(sock);
        return; // Quay lại menu nếu bind không thành công
    }

    // Set up destination address info
    memset(&sainfo, 0, sizeof(struct addrinfo));
    memset(&sin6, 0, sin6len); // Đặt toàn bộ sin6 về 0 lại.

    sainfo.ai_flags = 0;              // Không cài đặt cờ nào.
    sainfo.ai_family = PF_INET6;      // Địa chỉ là IPv6.
    sainfo.ai_socktype = SOCK_DGRAM;  // Kiểu socket là UDP.
    sainfo.ai_protocol = IPPROTO_UDP; // Sử dụng giao thức UDP.

    status = getaddrinfo(target_address, "3000", &sainfo, &psinfo);
    if (status != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        close(sock);
        return; // Quay lại menu nếu không lấy được địa chỉ
    }

    // Send command to target address
    status = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)psinfo->ai_addr, s_sin6len);
    if (status < 0)
    {
        perror("sendto");
        freeaddrinfo(psinfo);
        close(sock);
        return; // Quay lại menu nếu không gửi được lệnh
    }
    else
    {
        printf("Sent command: %s\n", buffer);
    }

    // Receive response
    s_status = recvfrom(sock, s_buffer, MAXBUF, 0, (struct sockaddr *)&s_sin6, &s_sin6len);
    if (s_status < 0)
    {
        perror("recvfrom");
        return;
    }
    else
    {
        printf("%s\n", s_buffer); // Replied message
    }

    // Clean up

    shutdown(sock, 2); // Đóng cả việc đọc và ghi trên socket.
    close(sock);       // Đóng socket hoàn toàn.

    // Giải phóng bộ nhớ đã cấp phát bởi getaddrinfo.
    freeaddrinfo(psinfo);
    psinfo = NULL; // Đặt psinfo về NULL để tránh con trỏ lơ lửng.
}

int main()
{
    int choice;
    display_menu();
    while (1)
    {
        printf("Enter your choice: ");
        scanf(" %d", &choice);

        switch (choice)
        {
        case 1:
            execute_command("1");
            break;
        case 2:
            execute_command("2");
            break;
        case 3:
            execute_command("3");
            break;
        case 4:
            execute_command("4");
            break;
        case 5:
            execute_command("5");
            break;
        case 6:
            execute_command("6");
            break;
        case 7:
            execute_command("7");
            break;
        case 8:
            execute_command("8");
            break;
        case 9:
            execute_command("9");
            break;
        case 10:
            execute_command("10");
            break;
        case 11:
            execute_command("11");
            break;
        case 12:
            execute_command("12");
            break;
        case 13:
            printf("Enter new target address: ");
            scanf("%s", target_address);
            printf("Target address set to %s\n", target_address);
            break;
        case 14:
            display_menu();
            break;
        case 15:
            printf("Exiting...\n");
            return 0;
        default:
            printf("Invalid command, please try again\n");
        }
    }
}

