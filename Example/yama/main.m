//
//  main.m
//  yama
//
//  Created by tsuiyuenhong on 11/30/2021.
//  Copyright (c) 2021 tsuiyuenhong. All rights reserved.
//

@import UIKit;
#import "YAMAAppDelegate.h"
#import "yama.h"

int main(int argc, char * argv[])
{
    yama_initialize();
    
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([YAMAAppDelegate class]));
    }
}
