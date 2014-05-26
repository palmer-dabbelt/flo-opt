/*
 * Copyright (C) 2014 Palmer Dabbelt
 *   <palmer.dabbelt@eecs.berkeley.edu>
 *
 * This file is part of flo-opt.
 *
 * flo-opt is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * flo-opt is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with flo-opt.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdio.h>
#include "pass_list.h++"
#include "pass_number.h++"
#include "flo.h++"
#include "version.h"

int main(int argc, const char **argv)
{
    if ((argc == 1) || ((argc == 2) && (strcmp(argv[1], "--help") == 0))) {
        printf("flo-opt <in.flo> <out.flo>: Optimizes Flo files\n");
        printf("  --help: Prints this help text\n");
        printf("  --version: Prints the version of this program in use\n");
        return (argc == 1) ? 0 : 1;
    }

    if ((argc == 2) && (strcmp(argv[1], "--version") == 0)) {
        printf("%s\n", PCONFIGURE_VERSION);
        return 0;
    }

    /* Reads a Flo file from the input file. */
    auto in_flo = flo::parse(argv[1]);

    /* Obtains the list of pass numbers and begins running each of
     * them in order. */
    std::shared_ptr<flo> cur = in_flo;
    for (const auto& pass_number: all_pass_numbers())
        for (const auto& pass: pass_list_lookup(pass_number))
            cur = pass->operate(cur);

    /* Just rename the flo file, it's now an output! */
    auto out_flo = cur;

    /* Creates a new output file and begins writing it out. */
    auto out_file = fopen(argv[2], "w");

    for (const auto& node : out_flo->nodes()) {
        if (!node->is_mem())
            continue;

        fprintf(out_file, "%s = mem'" SIZET_FORMAT " " SIZET_FORMAT "\n",
                node->name().c_str(),
                node->width(),
                node->depth()
            );
    }

    for (const auto& op : out_flo->operations()) {
        for (const auto& node: op->operands()) {
            if (node->known_width() == false) {
                fprintf(stderr, "Unknown width of node '%s' in '%s'\n",
                        node->name().c_str(),
                        op->to_string().c_str()
                    );
                abort();
            }
        }

        op->writeln(out_file);
    }

    fclose(out_file);

    return 0;
}
