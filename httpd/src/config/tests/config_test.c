#include <criterion/criterion.h>
#include <stdio.h>

#include "../config.h"

TestSuite(Config);

Test(Config, test_1)
{
    char *path = "tests/httpd.config";
    struct config *config = parse_configuration(path);
    cr_assert_str_eq(config->pid_file, "/tmp/HTTPd.pid");
    cr_assert_str_eq(config->log_file, "server.log");
    cr_assert_eq(config->log, true);
    cr_assert_eq(config->nb_servers, 1);
    for (size_t i = 0; i < config->nb_servers; i++)
    {
        cr_assert_str_eq(config->servers[0].ip, "127.0.0.1");
        cr_assert_str_eq(config->servers[0].port, "1312");
        cr_assert_str_eq(config->servers[0].root_dir, "votai/test.");
        cr_assert_str_eq(config->servers[0].default_file, "index.html");
        cr_assert_str_eq(config->servers[0].server_name->data, "images");
    }
    config_destroy(config);
}

Test(Config, error_path)
{
    char *path = "tests/httpd.confi";
    struct config *config = parse_configuration(path);
    cr_assert_eq(config, NULL);
}
Test(Config, error_global)
{
    char *path = "tests/httpd_global_error.config";
    struct config *config = parse_configuration(path);
    cr_assert_eq(config, NULL);
}

Test(Config, error_no_vhosts)
{
    char *path = "tests/httpd_no_vhosts_error.config";
    struct config *config = parse_configuration(path);
    cr_assert_eq(config, NULL);
}

Test(Config, error_vhosts_with_error)
{
    char *path = "tests/httpd_vhosts_with_error.config";
    struct config *config = parse_configuration(path);
    cr_assert_eq(config, NULL);
}

Test(Config, test_2)
{
    char *path = "tests/httpd_vhosts_multiple.config";
    struct config *config = parse_configuration(path);
    cr_assert_str_eq(config->pid_file, "/tmp/HTTPd.pid");
    cr_assert_str_eq(config->log_file, "server.log");
    cr_assert_eq(config->log, true);
    cr_assert_eq(config->nb_servers, 4);
    for (size_t i = 0; i < config->nb_servers; i++)
    {
        cr_assert_str_eq(config->servers[0].ip, "127.0.0.1");
        cr_assert_str_eq(config->servers[0].port, "1312");
        cr_assert_str_eq(config->servers[0].root_dir, "votai/test.");
        cr_assert_str_eq(config->servers[0].default_file, "index.html");
        cr_assert_str_eq(config->servers[0].server_name->data, "images");
    }
    config_destroy(config);
}
