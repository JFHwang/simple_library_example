#define RLBOX_SINGLE_THREADED_INVOCATIONS
#define RLBOX_USE_STATIC_CALLS() rlbox_noop_sandbox_lookup_symbol

#include "lib.h"
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "rlbox.hpp"
#include "rlbox_noop_sandbox.hpp"

using namespace rlbox;

static const char* PROGRAM_ERR_MSG [] = {
    "Succeeded",
    "Invalid image",
    "Incomplete Image",
    "Memory allocation failure"
};

#define MEMORY_ALLOC_MSG 3

void image_parsing_progress(rlbox_sandbox<rlbox_noop_sandbox>& _, tainted<unsigned int, rlbox_noop_sandbox> progress) {
    auto verifiedProgress = progress.copy_and_verify([](unsigned int val) {
        return val <= 100 ? val : 0;
    });
    std::cout << "Image parsing: " << verifiedProgress << " out of 100\n";
}

// An example application that simulates a typical image parsing program
// The library simulates a typilcal image decoding library such as libjpeg
int main(int argc, char const *argv[])
{
    rlbox::rlbox_sandbox<rlbox_noop_sandbox> sandbox;
    sandbox.create_sandbox();

    // create a buffer for input bytes
    char* input_stream = new char[100];
    if (!input_stream) {
        std::cerr << "Error: " << PROGRAM_ERR_MSG[MEMORY_ALLOC_MSG] << "\n";
        return 1;
    }

    // This is where we may read bytes from an image file into input_stream
    // But this is just a toy example
    // So we will just assume that input_stream buffer has bytes we want to parse

    auto tainted_input_stream = sandbox.malloc_in_sandbox<char>(100);
    auto taintedHeader = sandbox.invoke_sandbox_function(parse_image_header, tainted_input_stream);
    ImageHeader * header = (ImageHeader *)taintedHeader.copy_and_verify_address([](std::uintptr_t ptr) { return ptr; });

    if (header->status_code != STATUS_SUCCEEDED) {
        std::cerr << "Error: " << PROGRAM_ERR_MSG[header->status_code] << "\n";
        return 1;
    }

    char* output_stream = new char[header->height * header->width];
    if (!output_stream) {
        std::cerr << "Error: " << PROGRAM_ERR_MSG[MEMORY_ALLOC_MSG] << "\n";
        return 1;
    }

    auto tainted_output_stream = sandbox.malloc_in_sandbox<char>(header->height * header->width);
    auto progress_cb = sandbox.register_callback(image_parsing_progress);
    sandbox.invoke_sandbox_function(parse_image_body, tainted_input_stream, taintedHeader, progress_cb, tainted_output_stream);

    /*
    // Copy application's header structure into sandbox. In case of change
    auto taintedHeader2 = sandbox.malloc_in_sandbox<ImageHeader>();
    memcpy(taintedHeader2.unverified_safe_pointer_because(sizeof(ImageHeader), "tmp"), header, sizeof(ImageHeader));
    sandbox.invoke_sandbox_function(parse_image_body, tainted_input_stream, taintedHeader2, progress_cb, tainted_output_stream);
    */


    // Verify contents of tainted_output_stream and copy to output_stream
    memcpy(	output_stream, 
		tainted_output_stream.copy_and_verify_string([](std::unique_ptr<char[]> val) {
                    return val.strlen(val.get()) < header->height * header->weight ? std::move(val).get() : nullptr;
		}), 
		header->height * header->width);
    
    // Now verify tainted output stream and copy data over to local version
    // parse_image_body(input_stream, header, image_parsing_progress, output_stream);

    std::cout << "Image pixels: ";
    for (unsigned int i = 0; i < header->height; i++) {
        for (unsigned int j = 0; j < header->width; j++) {
            unsigned int index = i * header->width + j;
            std::cout << (unsigned int) output_stream[index] << " ";
        }
    }
    std::cout << "\n";

    free(header);
    delete input_stream;
    delete output_stream;
    sandbox.destroy_sandbox();

    return 0;
}
