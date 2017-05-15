//
//  ViewController.m
//  JDCFFPlayer
//
//  Created by maochengrui on 01/04/2017.
//  Copyright © 2017 jidong. All rights reserved.
//

#import "ViewController.h"
#import "JDCMediaPlayer.h"
#import "JDCGLView.h"

@interface ViewController ()
@property (weak, nonatomic) IBOutlet JDCGLView *glView;
@property (nonatomic,strong) JDCMediaPlayer *player;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    
    self.glView.backgroundColor = [UIColor blackColor];
    
    NSString *path = [[NSBundle mainBundle] pathForResource:@"war3end" ofType:@"mp4"];
    self.player = [[JDCMediaPlayer alloc] initWithURL:[NSURL fileURLWithPath:path]];
    [self.player addRenderer:self.glView];
    [self.player start];
}


- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}


@end
