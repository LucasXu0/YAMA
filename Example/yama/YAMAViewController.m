//
//  YAMAViewController.m
//  yama
//
//  Created by tsuiyuenhong on 11/30/2021.
//  Copyright (c) 2021 tsuiyuenhong. All rights reserved.
//

#import "YAMAViewController.h"
#import "yama.h"
#import "yama_system_info.h"

@interface YAMAViewController () <UITableViewDelegate, UITableViewDataSource>

@property (nonatomic, strong) UITableView *tableView;
@property (nonatomic, strong) NSArray<NSMutableArray <NSString *> *> *dataSource;
@property (nonatomic, strong) UIImageView *imageView;

@end

@implementation YAMAViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    
    self.dataSource = @[[NSMutableArray array], [NSMutableArray array]];
    [self.dataSource[0] addObject:@"malloc test"];
    [self.dataSource[0] addObject:@"image test"];
    
    [self.dataSource[1] addObject:@"start slow logging"];
    [self.dataSource[1] addObject:@"start fast logging"];
    
    self.tableView = [[UITableView alloc] initWithFrame:self.view.frame style:UITableViewStyleGrouped];
    [self.view addSubview:self.tableView];
    self.tableView.delegate = self;
    self.tableView.dataSource = self;
    [self.tableView registerClass:[UITableViewCell class] forCellReuseIdentifier:NSStringFromClass(self.class)];
}

- (void)testMalloc
{
    // malloc
    void *pointer = (void *)malloc(sizeof(int) * 102400);
    printf("[TEST] malloc a pointer = %p\n", pointer);
}

- (void)testImage
{
    self.imageView = [[UIImageView alloc] initWithFrame:self.view.frame];
    self.imageView.image = [UIImage imageNamed:@"sample.jpg"];
}

- (void)startSlowLogging
{
    yama_logging_context_t *context = (yama_logging_context_t *)malloc(sizeof(yama_logging_context_t));
    context->output_dir = (char *)NSTemporaryDirectory().UTF8String;
    context->mode = YAMA_LOGGING_MODE_SLOW;
    context->system_version = [[UIDevice currentDevice] systemVersion].UTF8String;
    context->system_arch = yama_get_device_arch();
    context->system_name = [[UIDevice currentDevice] systemName].UTF8String;
    yama_prepare_logging(context);
    
    double start = CFAbsoluteTimeGetCurrent();
    yama_start_logging();
    double end = CFAbsoluteTimeGetCurrent();
    printf("yama_start_logging cost %lfms\n", (end - start) * 1000.0);
        
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        yama_stop_logging();
        printf("😁 done\n");
    });
}

- (void)startFastLogging
{
    yama_logging_context_t *context = (yama_logging_context_t *)malloc(sizeof(yama_logging_context_t));
    context->output_dir = (char *)NSTemporaryDirectory().UTF8String;
    context->mode = YAMA_LOGGING_MODE_FAST;
    context->system_version = [[UIDevice currentDevice] systemVersion].UTF8String;
    context->system_arch = yama_get_device_arch();
    context->system_name = [[UIDevice currentDevice] systemName].UTF8String;
    yama_prepare_logging(context);
    
    double start = CFAbsoluteTimeGetCurrent();
    yama_start_logging();
    double end = CFAbsoluteTimeGetCurrent();
    printf("yama_start_logging cost %lfms\n", (end - start) * 1000.0);
        
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        yama_stop_logging();
        printf("😁 done\n");
    });
}

#pragma mark - UITableView
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return self.dataSource.count;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return self.dataSource[section].count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:NSStringFromClass(self.class) forIndexPath:indexPath];
    cell.textLabel.text = self.dataSource[indexPath.section][indexPath.row];
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSUInteger section = indexPath.section;
    NSUInteger row = indexPath.row;
    
    if (section == 0) {
        if (row == 0) {
            [self testMalloc];
        } else if (row == 1) {
            [self testImage];
        }
    } else if (section == 1) {
        if (row == 0) {
            [self startSlowLogging];
        } else if (row == 1) {
            [self startFastLogging];
        }
    }
    
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

@end
