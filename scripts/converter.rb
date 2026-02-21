#/usr/bin/env ruby

require 'pp'


File.open ARGV[0] do |file|
  items = file.read.split(/^:/)
  items = items.reject {|i| i.length == 0}.map { |i| i.gsub("\n", "\r") }

  pages = []
  current_page = []

  items.each do |i|
    if i.start_with?("---")
      pages.push current_page unless current_page.empty?
      current_page = [i.sub(/^---/, "")]
    else
      current_page.push(i)
    end
  end
  # push the last page
  pages.push current_page unless current_page.empty?

  puts "pres_data ="
  pp pages
  puts
end
