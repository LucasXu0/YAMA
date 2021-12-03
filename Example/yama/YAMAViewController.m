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

@property (nonatomic, strong) UIButton *loggingButton;
@property (nonatomic, strong) UIButton *increaseMemoryButton;

@property (nonatomic, strong) UIImage *image;

@end

@implementation YAMAViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    
    self.loggingButton = [[UIButton alloc] initWithFrame:CGRectMake(0, 100, self.view.frame.size.width, 100)];
    [self.view addSubview:self.loggingButton];
    [self.loggingButton addTarget:self action:@selector(startLogging) forControlEvents:UIControlEventTouchUpInside];
    [self.loggingButton setTitle:@"dump the memory" forState:UIControlStateNormal];
    [self.loggingButton setBackgroundColor:UIColor.greenColor];
    
    self.increaseMemoryButton = [[UIButton alloc] initWithFrame:CGRectMake(0, 250, self.view.frame.size.width, 100)];
    [self.view addSubview:self.increaseMemoryButton];
    [self.increaseMemoryButton addTarget:self action:@selector(increaseMemory) forControlEvents:UIControlEventTouchUpInside];
    [self.increaseMemoryButton setTitle:@"increase the memory" forState:UIControlStateNormal];
    [self.increaseMemoryButton setBackgroundColor:UIColor.redColor];
}

- (void)increaseMemory
{
    // image
    self.image = [UIImage imageNamed:@"sample"];
    printf("init sample image\n");
    
    // malloc
    void *pointer = (void *)malloc(sizeof(int) * 102400);
    printf("malloc a pointer = %p\n", pointer);
}

- (void)startLogging
{
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
