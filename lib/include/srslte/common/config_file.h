/**
 *
 * \section COPYRIGHT
 *
 * Copyright 2013-2015 Software Radio Systems Limited
 *
 * \section LICENSE
 *
 * This file is part of the srsUE library.
 *
 * srsUE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsUE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#ifndef SRSLTE_CONFIG_FILE_H
#define SRSLTE_CONFIG_FILE_H

#include <fstream>
#include <pwd.h>
#include "common.h"

static int cpfile(const char *to, const char *from)
{
  int fd_to, fd_from;
  char buf[4096];
  ssize_t nread;
  int saved_errno;

  fd_from = open(from, O_RDONLY);
  if (fd_from < 0)
    return -1;

  fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
  if (fd_to < 0)
    goto out_error;

  while (nread = read(fd_from, buf, sizeof buf), nread > 0) {
    char *out_ptr = buf;
    ssize_t nwritten;
    do {
      nwritten = write(fd_to, out_ptr, nread);
      if (nwritten >= 0) {
        nread -= nwritten;
        out_ptr += nwritten;
      }
      else if (errno != EINTR)
        goto out_error;
    } while (nread > 0);
  }

  if (nread == 0) {
    if (close(fd_to) < 0) {
      fd_to = -1;
      goto out_error;
    }
    close(fd_from);

    /* Success! */
    return 0;
  }

out_error:
  saved_errno = errno;

  close(fd_from);
  if (fd_to >= 0)
    close(fd_to);

  errno = saved_errno;
  return -1;
}




bool config_exists(std::string &filename, std::string default_name)
{
  std::ifstream conf(filename.c_str(), std::ios::in);
  if(conf.fail()) {
    const char *homedir = NULL;
    char full_path[256];
    ZERO_OBJECT(full_path);
    if ((homedir = getenv("HOME")) == NULL) {
      homedir = getpwuid(getuid())->pw_dir;
    }
    if (!homedir) {
      homedir = ".";
    }
    snprintf(full_path, sizeof(full_path), "%s/.srs/%s", homedir, default_name.c_str());
    filename = std::string(full_path);

    // try to open again
    conf.open(filename.c_str());
    if (conf.fail()) {
      std::string example_conf_path;
      example_conf_path = "/usr/share/srslte/" + default_name + ".example";
      FILE *file_exist_fp;
      file_exist_fp = fopen(example_conf_path.c_str());
      if (file_exist_fp != NULL) {
        fclose(file_exist_fp);
        fprintf(stderr, "No configuration file for %s found. Copying in default configuration from /usr/share/srslte/%s.example...", default_name.c_str(), default_name.c_str());
        std::string folder_name = std::str(homedir) + "/.srs";
        mkdir(folder_name.c_str(), 0777);
        cpfile(folder_name.c_str(), example_conf_path.c_str());
        fprintf(stderr, "Default configuration saved in %s/%s", folder_name.c_str(), default_name.c_str());
        return true;
      }
      return false;
    }
  }
  return true;
}

#endif // SRSLTE_CONFIG_FILE_H
