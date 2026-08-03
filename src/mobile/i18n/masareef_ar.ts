<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ar_EG" sourcelanguage="en">
<!--
  Arabic catalog, maintained by hand.

  Numerus messages carry six <numerusform> entries, in the order Qt uses for
  Arabic: zero, one, two, 3-10, 11-99, 100 and up. Getting the count wrong
  makes lrelease drop the message, so the plural rows are the first thing to
  check if a string turns up untranslated.

  No <location> elements: they only help Linguist jump to the source, and
  keeping line numbers accurate by hand is busywork that goes stale on the
  next edit.
-->
<context>
    <name>AddBillSheet</name>
    <message>
        <source>Add bill</source>
        <translation>إضافة فاتورة</translation>
    </message>
    <message>
        <source>Picked up where you left off.</source>
        <translation>تم استئناف ما بدأته.</translation>
    </message>
    <message>
        <source>Start over</source>
        <translation>البدء من جديد</translation>
    </message>
</context>
<context>
    <name>AddExpenseSheet</name>
    <message>
        <source>Add expense</source>
        <translation>إضافة مصروف</translation>
    </message>
    <message>
        <source>Picked up where you left off.</source>
        <translation>تم استئناف ما بدأته.</translation>
    </message>
    <message>
        <source>Start over</source>
        <translation>البدء من جديد</translation>
    </message>
    <message>
        <source>Filled in from the price book.</source>
        <translation>تمت التعبئة من دفتر الأسعار.</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>مسح</translation>
    </message>
</context>
<context>
    <name>AddPriceItemSheet</name>
    <message>
        <source>Add item</source>
        <translation>إضافة صنف</translation>
    </message>
    <message>
        <source>Picked up where you left off.</source>
        <translation>تم استئناف ما بدأته.</translation>
    </message>
    <message>
        <source>Start over</source>
        <translation>البدء من جديد</translation>
    </message>
    <message>
        <source>Copied from an existing item. Give it its own name.</source>
        <translation>نسخة من صنف موجود. اختر له اسمًا مختلفًا.</translation>
    </message>
    <message>
        <source>Clear</source>
        <translation>مسح</translation>
    </message>
</context>
<context>
    <name>AmountField</name>
    <message>
        <source>Amount</source>
        <translation>المبلغ</translation>
    </message>
</context>
<context>
    <name>BillController</name>
    <message>
        <source>Please enter a valid amount greater than zero.</source>
        <translation>من فضلك أدخل مبلغًا صحيحًا أكبر من صفر.</translation>
    </message>
    <message>
        <source>Please choose a category.</source>
        <translation>من فضلك اختر فئة.</translation>
    </message>
    <message>
        <source>Please choose a due date.</source>
        <translation>من فضلك اختر تاريخ الاستحقاق.</translation>
    </message>
    <message>
        <source>Please enter a bill name.</source>
        <translation>من فضلك أدخل اسم الفاتورة.</translation>
    </message>
</context>
<context>
    <name>BillDelegate</name>
    <message>
        <source>Paused</source>
        <translation>موقوفة</translation>
    </message>
    <message>
        <source>Paid</source>
        <translation>تم الدفع</translation>
    </message>
</context>
<context>
    <name>BillForm</name>
    <message>
        <source>Bill name</source>
        <translation>اسم الفاتورة</translation>
    </message>
    <message>
        <source>Category</source>
        <translation>الفئة</translation>
    </message>
    <message>
        <source>Repeats</source>
        <translation>التكرار</translation>
    </message>
    <message>
        <source>Monthly</source>
        <translation>شهريًا</translation>
    </message>
    <message>
        <source>Quarterly</source>
        <translation>ربع سنوي</translation>
    </message>
    <message>
        <source>Yearly</source>
        <translation>سنويًا</translation>
    </message>
    <message>
        <source>Next due</source>
        <translation>الاستحقاق القادم</translation>
    </message>
    <message>
        <source>Notes (optional)</source>
        <translation>ملاحظات (اختياري)</translation>
    </message>
</context>
<context>
    <name>BillRepository</name>
    <message>
        <source>Recurring bill %1 not found</source>
        <translation>الفاتورة الدورية %1 غير موجودة</translation>
    </message>
</context>
<context>
    <name>BillsScreen</name>
    <message>
        <source>Edit</source>
        <translation>تعديل</translation>
    </message>
    <message>
        <source>Mark paid</source>
        <translation>تسجيل الدفع</translation>
    </message>
    <message>
        <source>Pause</source>
        <translation>إيقاف مؤقت</translation>
    </message>
    <message>
        <source>Resume</source>
        <translation>استئناف</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>حذف</translation>
    </message>
    <message>
        <source>Logged %1</source>
        <translation>تم تسجيل %1</translation>
    </message>
    <message>
        <source>%1 resumed</source>
        <translation>تم استئناف %1</translation>
    </message>
    <message>
        <source>%1 paused</source>
        <translation>تم إيقاف %1 مؤقتًا</translation>
    </message>
    <message>
        <source>Delete this bill?</source>
        <translation>حذف هذه الفاتورة؟</translation>
    </message>
    <message>
        <source>Expenses already logged for it are kept.</source>
        <translation>تبقى المصروفات المسجَّلة لها كما هي.</translation>
    </message>
    <message>
        <source>Bills each month</source>
        <translation>الفواتير شهريًا</translation>
    </message>
    <message numerus="yes">
        <source>%n active bill(s), spread evenly</source>
        <translation>
            <numerusform>لا توجد فواتير نشطة، موزَّعة بالتساوي</numerusform>
            <numerusform>فاتورة نشطة واحدة، موزَّعة بالتساوي</numerusform>
            <numerusform>فاتورتان نشطتان، موزَّعة بالتساوي</numerusform>
            <numerusform>%n فواتير نشطة، موزَّعة بالتساوي</numerusform>
            <numerusform>%n فاتورة نشطة، موزَّعة بالتساوي</numerusform>
            <numerusform>%n فاتورة نشطة، موزَّعة بالتساوي</numerusform>
        </translation>
    </message>
    <message>
        <source>Active</source>
        <translation>النشطة</translation>
    </message>
    <message>
        <source>All</source>
        <translation>الكل</translation>
    </message>
    <message>
        <source>Overdue</source>
        <translation>متأخرة</translation>
    </message>
    <message>
        <source>Due soon</source>
        <translation>قريبة الاستحقاق</translation>
    </message>
    <message>
        <source>This month</source>
        <translation>هذا الشهر</translation>
    </message>
    <message>
        <source>Paused</source>
        <translation>موقوفة</translation>
    </message>
    <message>
        <source>Later</source>
        <translation>لاحقًا</translation>
    </message>
    <message>
        <source>No bills yet</source>
        <translation>لا توجد فواتير بعد</translation>
    </message>
    <message>
        <source>Tap + to track a bill that comes back every month</source>
        <translation>اضغط + لمتابعة فاتورة تتكرر كل شهر</translation>
    </message>
</context>
<context>
    <name>BottomSheet</name>
    <message>
        <source>Save</source>
        <translation>حفظ</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>إلغاء</translation>
    </message>
</context>
<context>
    <name>BudgetViewModel</name>
    <message>
        <source>Please enter a valid amount greater than zero.</source>
        <translation>من فضلك أدخل مبلغًا صحيحًا أكبر من صفر.</translation>
    </message>
</context>
<context>
    <name>BudgetsScreen</name>
    <message>
        <source>Monthly budget</source>
        <translation>الميزانية الشهرية</translation>
    </message>
    <message>
        <source>No budget set</source>
        <translation>لم تُحدَّد ميزانية</translation>
    </message>
    <message>
        <source>Category budgets</source>
        <translation>ميزانيات الفئات</translation>
    </message>
</context>
<context>
    <name>CategoryManagerScreen</name>
    <message>
        <source>Categories</source>
        <translation>الفئات</translation>
    </message>
    <message>
        <source>New category</source>
        <translation>فئة جديدة</translation>
    </message>
    <message>
        <source>Edit category</source>
        <translation>تعديل الفئة</translation>
    </message>
    <message>
        <source>Name</source>
        <translation>الاسم</translation>
    </message>
    <message>
        <source>Move expenses from &quot;%1&quot;</source>
        <translation>نقل المصروفات من &quot;%1&quot;</translation>
    </message>
    <message>
        <source>This category is used by existing expenses or recurring bills. Choose where to move them:</source>
        <translation>هذه الفئة مستخدمة في مصروفات أو فواتير دورية قائمة. اختر إلى أين تنقلها:</translation>
    </message>
</context>
<context>
    <name>CategoryPicker</name>
    <message>
        <source>All</source>
        <translation>الكل</translation>
    </message>
    <message>
        <source>Uncategorized</source>
        <translation>بدون فئة</translation>
    </message>
</context>
<context>
    <name>CategoryRepository</name>
    <message>
        <source>The category name cannot be empty.</source>
        <translation>لا يمكن ترك اسم الفئة فارغًا.</translation>
    </message>
    <message>
        <source>Could not add &quot;%1&quot; — a category with this name may already exist.</source>
        <translation>تعذَّرت إضافة &quot;%1&quot; — قد توجد فئة بهذا الاسم بالفعل.</translation>
    </message>
    <message>
        <source>Could not rename — a category named &quot;%1&quot; may already exist.</source>
        <translation>تعذَّرت إعادة التسمية — قد توجد فئة باسم &quot;%1&quot; بالفعل.</translation>
    </message>
    <message>
        <source>Category not found.</source>
        <translation>الفئة غير موجودة.</translation>
    </message>
    <message>
        <source>At least one category must remain.</source>
        <translation>يجب أن تبقى فئة واحدة على الأقل.</translation>
    </message>
    <message>
        <source>&quot;%1&quot; is used by existing expenses or recurring bills. Reassign or delete those entries first.</source>
        <translation>&quot;%1&quot; مستخدمة في مصروفات أو فواتير دورية قائمة. انقل تلك السجلات أو احذفها أولًا.</translation>
    </message>
    <message>
        <source>Cannot reassign a category to itself.</source>
        <translation>لا يمكن نقل الفئة إلى نفسها.</translation>
    </message>
</context>
<context>
    <name>ConfirmDialog</name>
    <message>
        <source>OK</source>
        <translation>موافق</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation>إلغاء</translation>
    </message>
</context>
<context>
    <name>DateField</name>
    <message>
        <source>Today</source>
        <translation>اليوم</translation>
    </message>
</context>
<context>
    <name>EditBillSheet</name>
    <message>
        <source>Edit bill</source>
        <translation>تعديل الفاتورة</translation>
    </message>
    <message>
        <source>Delete this bill?</source>
        <translation>حذف هذه الفاتورة؟</translation>
    </message>
    <message>
        <source>Expenses already logged for it are kept.</source>
        <translation>تبقى المصروفات المسجَّلة لها كما هي.</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>حذف</translation>
    </message>
</context>
<context>
    <name>EditExpenseSheet</name>
    <message>
        <source>Edit expense</source>
        <translation>تعديل المصروف</translation>
    </message>
    <message>
        <source>Delete this expense?</source>
        <translation>حذف هذا المصروف؟</translation>
    </message>
    <message>
        <source>This cannot be undone.</source>
        <translation>لا يمكن التراجع عن هذا.</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>حذف</translation>
    </message>
</context>
<context>
    <name>EditPriceItemSheet</name>
    <message>
        <source>Edit item</source>
        <translation>تعديل الصنف</translation>
    </message>
    <message>
        <source>Delete this item?</source>
        <translation>حذف هذا الصنف؟</translation>
    </message>
    <message>
        <source>Its price history is deleted with it.</source>
        <translation>يُحذف معه سجل أسعاره.</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>حذف</translation>
    </message>
    <message>
        <source>Was %1</source>
        <translation>كان %1</translation>
    </message>
</context>
<context>
    <name>ExpenseController</name>
    <message>
        <source>Please enter a valid amount greater than zero.</source>
        <translation>من فضلك أدخل مبلغًا صحيحًا أكبر من صفر.</translation>
    </message>
    <message>
        <source>Please choose a category.</source>
        <translation>من فضلك اختر فئة.</translation>
    </message>
    <message>
        <source>Please choose a date.</source>
        <translation>من فضلك اختر تاريخًا.</translation>
    </message>
</context>
<context>
    <name>ExpenseForm</name>
    <message>
        <source>Category</source>
        <translation>الفئة</translation>
    </message>
    <message>
        <source>Date</source>
        <translation>التاريخ</translation>
    </message>
    <message>
        <source>Today</source>
        <translation>اليوم</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>الوصف</translation>
    </message>
    <message>
        <source>Notes (optional)</source>
        <translation>ملاحظات (اختياري)</translation>
    </message>
</context>
<context>
    <name>ExpenseModel</name>
    <message>
        <source>Date</source>
        <translation>التاريخ</translation>
    </message>
    <message>
        <source>Category</source>
        <translation>الفئة</translation>
    </message>
    <message>
        <source>Description</source>
        <translation>الوصف</translation>
    </message>
    <message>
        <source>Amount</source>
        <translation>المبلغ</translation>
    </message>
    <message>
        <source>Notes</source>
        <translation>ملاحظات</translation>
    </message>
</context>
<context>
    <name>ExpenseRepository</name>
    <message>
        <source>Expense %1 not found</source>
        <translation>المصروف %1 غير موجود</translation>
    </message>
</context>
<context>
    <name>ExpensesScreen</name>
    <message>
        <source>Expense deleted</source>
        <translation>تم حذف المصروف</translation>
    </message>
    <message>
        <source>Undo</source>
        <translation>تراجع</translation>
    </message>
    <message>
        <source>Edit</source>
        <translation>تعديل</translation>
    </message>
    <message>
        <source>Duplicate</source>
        <translation>تكرار</translation>
    </message>
    <message>
        <source>Select</source>
        <translation>تحديد</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>حذف</translation>
    </message>
    <message>
        <source>Duplicated</source>
        <translation>تم التكرار</translation>
    </message>
    <message numerus="yes">
        <source>Delete %n expense(s)?</source>
        <translation>
            <numerusform>حذف المصروفات؟</numerusform>
            <numerusform>حذف مصروف واحد؟</numerusform>
            <numerusform>حذف مصروفين؟</numerusform>
            <numerusform>حذف %n مصروفات؟</numerusform>
            <numerusform>حذف %n مصروفًا؟</numerusform>
            <numerusform>حذف %n مصروف؟</numerusform>
        </translation>
    </message>
    <message>
        <source>This cannot be undone.</source>
        <translation>لا يمكن التراجع عن هذا.</translation>
    </message>
    <message>
        <source>Search expenses</source>
        <translation>ابحث في المصروفات</translation>
    </message>
    <message>
        <source>Hide date filter</source>
        <translation>إخفاء تصفية التاريخ</translation>
    </message>
    <message>
        <source>Show date filter</source>
        <translation>إظهار تصفية التاريخ</translation>
    </message>
    <message>
        <source>From</source>
        <translation>من</translation>
    </message>
    <message>
        <source>To</source>
        <translation>إلى</translation>
    </message>
    <message>
        <source>Clear dates</source>
        <translation>مسح التواريخ</translation>
    </message>
    <message>
        <source>Total: %1</source>
        <translation>الإجمالي: %1</translation>
    </message>
    <message numerus="yes">
        <source>%n expense(s)</source>
        <translation>
            <numerusform>لا توجد مصروفات</numerusform>
            <numerusform>مصروف واحد</numerusform>
            <numerusform>مصروفان</numerusform>
            <numerusform>%n مصروفات</numerusform>
            <numerusform>%n مصروفًا</numerusform>
            <numerusform>%n مصروف</numerusform>
        </translation>
    </message>
    <message>
        <source>Nothing here</source>
        <translation>لا يوجد شيء هنا</translation>
    </message>
    <message>
        <source>No expenses match your filters</source>
        <translation>لا توجد مصروفات تطابق التصفية</translation>
    </message>
</context>
<context>
    <name>HomeScreen</name>
    <message>
        <source>Spent this month</source>
        <translation>المصروف هذا الشهر</translation>
    </message>
    <message>
        <source>Over budget</source>
        <translation>تجاوزت الميزانية</translation>
    </message>
    <message>
        <source>%1 left of %2</source>
        <translation>بقي %1 من %2</translation>
    </message>
    <message>
        <source>View Reports</source>
        <translation>عرض التقارير</translation>
    </message>
    <message>
        <source>Top categories</source>
        <translation>أكثر الفئات إنفاقًا</translation>
    </message>
    <message>
        <source>Recent expenses</source>
        <translation>أحدث المصروفات</translation>
    </message>
    <message>
        <source>No expenses yet</source>
        <translation>لا توجد مصروفات بعد</translation>
    </message>
    <message>
        <source>Tap + to add your first expense</source>
        <translation>اضغط + لإضافة أول مصروف</translation>
    </message>
</context>
<context>
    <name>LocaleFormat</name>
    <message>
        <source>Today</source>
        <translation>اليوم</translation>
    </message>
    <message>
        <source>Yesterday</source>
        <translation>أمس</translation>
    </message>
    <message>
        <source>Tomorrow</source>
        <translation>غدًا</translation>
    </message>
    <message>
        <source>Due today</source>
        <translation>مستحقة اليوم</translation>
    </message>
    <message>
        <source>Due tomorrow</source>
        <translation>مستحقة غدًا</translation>
    </message>
    <message numerus="yes">
        <source>Due in %n day(s)</source>
        <translation>
            <numerusform>مستحقة اليوم</numerusform>
            <numerusform>مستحقة بعد يوم</numerusform>
            <numerusform>مستحقة بعد يومين</numerusform>
            <numerusform>مستحقة بعد %n أيام</numerusform>
            <numerusform>مستحقة بعد %n يومًا</numerusform>
            <numerusform>مستحقة بعد %n يوم</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <source>Overdue by %n day(s)</source>
        <translation>
            <numerusform>متأخرة</numerusform>
            <numerusform>متأخرة بيوم</numerusform>
            <numerusform>متأخرة بيومين</numerusform>
            <numerusform>متأخرة بـ %n أيام</numerusform>
            <numerusform>متأخرة بـ %n يومًا</numerusform>
            <numerusform>متأخرة بـ %n يوم</numerusform>
        </translation>
    </message>
</context>
<context>
    <name>Main</name>
    <message>
        <source>Masareef</source>
        <translation>مصاريف</translation>
    </message>
    <message>
        <source>Reports</source>
        <translation>التقارير</translation>
    </message>
</context>
<context>
    <name>NavBar</name>
    <message>
        <source>Home</source>
        <translation>الرئيسية</translation>
    </message>
    <message>
        <source>Expenses</source>
        <translation>المصروفات</translation>
    </message>
    <message>
        <source>Bills</source>
        <translation>الفواتير</translation>
    </message>
    <message>
        <source>Prices</source>
        <translation>الأسعار</translation>
    </message>
    <message>
        <source>Budgets</source>
        <translation>الميزانيات</translation>
    </message>
    <message>
        <source>Settings</source>
        <translation>الإعدادات</translation>
    </message>
</context>
<context>
    <name>PriceBookScreen</name>
    <message>
        <source>Edit</source>
        <translation>تعديل</translation>
    </message>
    <message>
        <source>Log as expense</source>
        <translation>تسجيل كمصروف</translation>
    </message>
    <message>
        <source>Duplicate</source>
        <translation>تكرار</translation>
    </message>
    <message>
        <source>Delete</source>
        <translation>حذف</translation>
    </message>
    <message>
        <source>%1 (copy)</source>
        <translation>%1 (نسخة)</translation>
    </message>
    <message>
        <source>Delete this item?</source>
        <translation>حذف هذا الصنف؟</translation>
    </message>
    <message>
        <source>Its price history is deleted with it.</source>
        <translation>يُحذف معه سجل أسعاره.</translation>
    </message>
    <message>
        <source>Search the price book</source>
        <translation>ابحث في دفتر الأسعار</translation>
    </message>
    <message numerus="yes">
        <source>%n item(s)</source>
        <translation>
            <numerusform>لا توجد أصناف</numerusform>
            <numerusform>صنف واحد</numerusform>
            <numerusform>صنفان</numerusform>
            <numerusform>%n أصناف</numerusform>
            <numerusform>%n صنفًا</numerusform>
            <numerusform>%n صنف</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <source>%n dearer than before</source>
        <translation>
            <numerusform>لا شيء أغلى من قبل</numerusform>
            <numerusform>صنف واحد أغلى من قبل</numerusform>
            <numerusform>صنفان أغلى من قبل</numerusform>
            <numerusform>%n أصناف أغلى من قبل</numerusform>
            <numerusform>%n صنفًا أغلى من قبل</numerusform>
            <numerusform>%n صنف أغلى من قبل</numerusform>
        </translation>
    </message>
    <message numerus="yes">
        <source>%n cheaper than before</source>
        <translation>
            <numerusform>لا شيء أرخص من قبل</numerusform>
            <numerusform>صنف واحد أرخص من قبل</numerusform>
            <numerusform>صنفان أرخص من قبل</numerusform>
            <numerusform>%n أصناف أرخص من قبل</numerusform>
            <numerusform>%n صنفًا أرخص من قبل</numerusform>
            <numerusform>%n صنف أرخص من قبل</numerusform>
        </translation>
    </message>
    <message>
        <source>%1 dearer, %2 cheaper</source>
        <translation>%1 أغلى، %2 أرخص</translation>
    </message>
    <message>
        <source>Nothing found</source>
        <translation>لا توجد نتائج</translation>
    </message>
    <message>
        <source>No prices yet</source>
        <translation>لا توجد أسعار بعد</translation>
    </message>
    <message>
        <source>No item matches your search</source>
        <translation>لا يوجد صنف يطابق بحثك</translation>
    </message>
    <message>
        <source>Tap + to record what something costs</source>
        <translation>اضغط + لتسجيل سعر أي شيء</translation>
    </message>
</context>
<context>
    <name>PriceItemController</name>
    <message>
        <source>Please enter an item name.</source>
        <translation>من فضلك أدخل اسم الصنف.</translation>
    </message>
    <message>
        <source>Please enter a valid price.</source>
        <translation>من فضلك أدخل سعرًا صحيحًا.</translation>
    </message>
</context>
<context>
    <name>PriceItemDelegate</name>
    <message>
        <source>per %1</source>
        <translation>لكل %1</translation>
    </message>
</context>
<context>
    <name>PriceItemForm</name>
    <message>
        <source>Item name</source>
        <translation>اسم الصنف</translation>
    </message>
    <message>
        <source>Price</source>
        <translation>السعر</translation>
    </message>
    <message>
        <source>Unit</source>
        <translation>الوحدة</translation>
    </message>
    <message>
        <source>kg, litre, piece…</source>
        <translation>كجم، لتر، قطعة…</translation>
    </message>
    <message>
        <source>kg</source>
        <translation>كجم</translation>
    </message>
    <message>
        <source>g</source>
        <translation>جم</translation>
    </message>
    <message>
        <source>litre</source>
        <translation>لتر</translation>
    </message>
    <message>
        <source>ml</source>
        <translation>مل</translation>
    </message>
    <message>
        <source>piece</source>
        <translation>قطعة</translation>
    </message>
    <message>
        <source>pack</source>
        <translation>عبوة</translation>
    </message>
    <message>
        <source>box</source>
        <translation>علبة</translation>
    </message>
    <message>
        <source>dozen</source>
        <translation>دستة</translation>
    </message>
    <message>
        <source>Category</source>
        <translation>الفئة</translation>
    </message>
</context>
<context>
    <name>PriceItemRepository</name>
    <message>
        <source>The item name cannot be empty.</source>
        <translation>لا يمكن ترك اسم الصنف فارغًا.</translation>
    </message>
    <message>
        <source>The price cannot be negative.</source>
        <translation>لا يمكن أن يكون السعر بالسالب.</translation>
    </message>
    <message>
        <source>Price book item %1 not found</source>
        <translation>الصنف %1 غير موجود في دفتر الأسعار</translation>
    </message>
    <message>
        <source>Could not add &quot;%1&quot; — the price book may already have an item with this name.</source>
        <translation>تعذَّرت إضافة &quot;%1&quot; — قد يوجد صنف بهذا الاسم في دفتر الأسعار بالفعل.</translation>
    </message>
    <message>
        <source>Could not save &quot;%1&quot; — the price book may already have an item with this name.</source>
        <translation>تعذَّر حفظ &quot;%1&quot; — قد يوجد صنف بهذا الاسم في دفتر الأسعار بالفعل.</translation>
    </message>
</context>
<context>
    <name>Recurrence</name>
    <message>
        <source>Monthly</source>
        <translation>شهريًا</translation>
    </message>
    <message>
        <source>Quarterly</source>
        <translation>ربع سنوي</translation>
    </message>
    <message>
        <source>Yearly</source>
        <translation>سنويًا</translation>
    </message>
</context>
<context>
    <name>ReportsScreen</name>
    <message>
        <source>Reports</source>
        <translation>التقارير</translation>
    </message>
    <message>
        <source>Monthly Totals (Last 12 Months)</source>
        <translation>الإجماليات الشهرية (آخر 12 شهرًا)</translation>
    </message>
    <message>
        <source>Categories (Current Month)</source>
        <translation>الفئات (الشهر الحالي)</translation>
    </message>
</context>
<context>
    <name>SearchField</name>
    <message>
        <source>Search</source>
        <translation>بحث</translation>
    </message>
</context>
<context>
    <name>SettingsScreen</name>
    <message>
        <source>Restore from backup?</source>
        <translation>الاستعادة من نسخة احتياطية؟</translation>
    </message>
    <message>
        <source>Backup restored successfully</source>
        <translation>تمت الاستعادة بنجاح</translation>
    </message>
    <message>
        <source>Restore failed</source>
        <translation>فشلت الاستعادة</translation>
    </message>
    <message>
        <source>This will replace your current data with the selected backup. Continue?</source>
        <translation>سيحل هذا محل بياناتك الحالية بالنسخة المحددة. هل تريد المتابعة؟</translation>
    </message>
    <message>
        <source>Backup</source>
        <translation>نسخة احتياطية</translation>
    </message>
    <message>
        <source>Appearance</source>
        <translation>المظهر</translation>
    </message>
    <message>
        <source>System</source>
        <translation>النظام</translation>
    </message>
    <message>
        <source>Light</source>
        <translation>فاتح</translation>
    </message>
    <message>
        <source>Dark</source>
        <translation>داكن</translation>
    </message>
    <message>
        <source>Language</source>
        <translation>اللغة</translation>
    </message>
    <message>
        <source>Currency</source>
        <translation>العملة</translation>
    </message>
    <message>
        <source>Backup &amp; Restore</source>
        <translation>النسخ الاحتياطي والاستعادة</translation>
    </message>
    <message>
        <source>Back up now</source>
        <translation>إنشاء نسخة الآن</translation>
    </message>
    <message>
        <source>Backup created successfully</source>
        <translation>تم إنشاء النسخة بنجاح</translation>
    </message>
    <message>
        <source>Backup failed</source>
        <translation>فشل إنشاء النسخة</translation>
    </message>
    <message>
        <source>Recent backups</source>
        <translation>أحدث النسخ الاحتياطية</translation>
    </message>
    <message>
        <source>Categories</source>
        <translation>الفئات</translation>
    </message>
    <message>
        <source>Manage categories</source>
        <translation>إدارة الفئات</translation>
    </message>
    <message>
        <source>About</source>
        <translation>حول التطبيق</translation>
    </message>
    <message>
        <source>Masareef %1 — a simple expense tracker</source>
        <translation>مصاريف %1 — تطبيق بسيط لتتبع المصروفات</translation>
    </message>
</context>
</TS>
