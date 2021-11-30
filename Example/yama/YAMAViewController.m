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
    
    double start = CFAbsoluteTimeGetCurrent();
    yama_start_logging();
    double end = CFAbsoluteTimeGetCurrent();
    printf("yama_start_logging cost %lfms\n", (end - start) * 1000.0);
    yama_stop_logging();
}

@end
