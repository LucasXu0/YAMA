//
//  YAMAViewController.m
//  yama
//
//  Created by tsuiyuenhong on 11/30/2021.
//  Copyright (c) 2021 tsuiyuenhong. All rights reserved.
//

#import "YAMAViewController.h"
#import "yama.h"

@interface YAMAViewController ()

@end

@implementation YAMAViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    
    // [[NSFileManager defaultManager] removeItemAtPath:NSTemporaryDirectory() error:nil];
    
    yama_logging_context_t *context = (yama_logging_context_t *)malloc(sizeof(yama_logging_context_t));
    context->output_logging_file_path = [NSTemporaryDirectory() stringByAppendingString:@"msl.log"].UTF8String;
    context->output_headers_file_path = [NSTemporaryDirectory() stringByAppendingString:@"header.log"].UTF8String;
    context->output_serialize_table_file_path = [NSTemporaryDirectory() stringByAppendingString:@"serialize_table"].UTF8String;
    yama_prepare_logging(context);
    
    double start = CFAbsoluteTimeGetCurrent();
    yama_start_logging();
    double end = CFAbsoluteTimeGetCurrent();
    printf("yama_start_logging cost %lfms\n", (end - start) * 1000.0);
    
    dump_headers();
    
    yama_stop_logging();
    
    
}

@end
