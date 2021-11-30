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
    
    yama_start_logging();
    yama_stop_logging();
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
