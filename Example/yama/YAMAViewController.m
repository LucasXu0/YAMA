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
    
    [[NSFileManager defaultManager] removeItemAtPath:NSTemporaryDirectory() error:nil];
    [[NSFileManager defaultManager] createDirectoryAtPath:NSTemporaryDirectory() withIntermediateDirectories:YES attributes:nil error:nil];
    
    yama_logging_context_t *context = (yama_logging_context_t *)malloc(sizeof(yama_logging_context_t));
    context->output_dir = (char *)NSTemporaryDirectory().UTF8String;
    yama_prepare_logging(context);
    
    double start = CFAbsoluteTimeGetCurrent();
    yama_start_logging();
    double end = CFAbsoluteTimeGetCurrent();
    printf("yama_start_logging cost %lfms\n", (end - start) * 1000.0);
        
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        yama_stop_logging();
        printf("😁 done\n");
    });
}

@end
