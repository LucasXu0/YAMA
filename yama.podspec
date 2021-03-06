#
# Be sure to run `pod lib lint yama.podspec' to ensure this is a
# valid spec before submitting.
#
# Any lines starting with a # are optional, but their use is encouraged
# To learn more about a Podspec see https://guides.cocoapods.org/syntax/podspec.html
#

Pod::Spec.new do |s|
  s.name             = 'yama'
  s.version          = '0.0.2'
  s.summary          = 'A offline and advanced memory analysis tool based on MallocStackLogging.'

# This description is used to generate tags and improve search results.
#   * Think: What does it do? Why did you write it? What is the focus?
#   * Try to keep it short, snappy and to the point.
#   * Write the description between the DESC delimiters below.
#   * Finally, don't worry about the indent, CocoaPods strips it!

  s.description      = <<-DESC
  A offline and advanced memory analysis tool based on MallocStackLogging.
                       DESC

  s.homepage         = 'https://github.com/tsuiyuenhong/yama'
  s.license          = { :type => 'MIT', :file => 'LICENSE' }
  s.author           = { 'tsuiyuenhong' => 'tsuiyuenhong@gmail.com' }
  s.source           = { :git => 'https://github.com/tsuiyuenhong/yama.git', :tag => s.version.to_s }

  s.ios.deployment_target = '10.0'

  s.source_files = 'yama/**/*'
end
